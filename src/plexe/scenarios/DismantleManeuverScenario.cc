//
// Copyright (C) 2012-2021 Michele Segata <segata@ccs-labs.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "plexe/scenarios/DismantleManeuverScenario.h"

namespace plexe {

Define_Module(DismantleManeuverScenario)

void DismantleManeuverScenario::initialize(int stage)
{
    BaseScenario::initialize(stage);

    if (stage == 1) {
        app = FindModule<DismantlePlatooningApp*>::findSubModule(getParentModule());
        prepareManeuverCars(0);
    }
}

void DismantleManeuverScenario::prepareManeuverCars(int platoonLane)
{
    ACTIVE_CONTROLLER leaderController = getEnum(par("leaderController").stringValue());
    ACTIVE_CONTROLLER followerController = getEnum(par("followerController").stringValue());
    ACTIVE_CONTROLLER otherCarController = getEnum(par("otherCarController").stringValue());

    const int numberOfCars = par("numberOfCars").intValue();
    const int numberOfCarsPerPlatoon = par("numberOfCarsPerPlatoon").intValue();

    const int meneuverTime = par("meneuverTime").intValue();

    const int desiredPlatoonSpeed = par("desiredPlatoonSpeed").intValue();
    const int desiredOtherCarSpeed = par("desiredOtherCarSpeed").intValue();

    if (positionHelper->getId()%numberOfCarsPerPlatoon == 0 && positionHelper->getId() < numberOfCars) {
        // this is the leader of the platoon ahead
        plexeTraciVehicle->setCruiseControlDesiredSpeed(desiredPlatoonSpeed / 3.6);
        plexeTraciVehicle->setActiveController(leaderController);
        plexeTraciVehicle->setFixedLane(platoonLane);
        app->setPlatoonRole(PlatoonRole::LEADER);

        // after 3 seconds of simulation, start the maneuver
        // TODO CHANGE FOR DESTROY
        startManeuver = new cMessage();
        destroyManeuver = new cMessage();
        scheduleAt(simTime() + SimTime(meneuverTime), destroyManeuver);
    }
    else if (!positionHelper->isLeader()) {
        if (positionHelper->getId() < numberOfCars)
        {
            // these are the followers which are already in the platoon
            plexeTraciVehicle->setCruiseControlDesiredSpeed(desiredPlatoonSpeed / 3.6);
            plexeTraciVehicle->setActiveController(followerController);
            plexeTraciVehicle->setFixedLane(platoonLane); // all the platoons will be on the first lane
            app->setPlatoonRole(PlatoonRole::FOLLOWER);
        }
        else {
            plexeTraciVehicle->setCruiseControlDesiredSpeed(desiredOtherCarSpeed / 3.6);
            plexeTraciVehicle->setActiveController(otherCarController);
            plexeTraciVehicle->setFixedLane(1);
        }
    }
}

DismantleManeuverScenario::~DismantleManeuverScenario()
{
    cancelAndDelete(startManeuver);
    startManeuver = nullptr;
    cancelAndDelete(destroyManeuver);
    destroyManeuver = nullptr;
}

void DismantleManeuverScenario::handleSelfMsg(cMessage* msg)
{
    BaseScenario::handleSelfMsg(msg);
    if (msg == startManeuver) app->startDismantleManeuver(0, 0);
    if (msg == destroyManeuver) app->startDismantleManeuver(1, 0);
}

ACTIVE_CONTROLLER DismantleManeuverScenario::getEnum(const char* str)
{
    if (strcmp(str, "ACC") == 0) {
        return ACC;
    } else if (strcmp(str, "CACC") == 0) {
        return CACC;
    } else if (strcmp(str, "PLOEG") == 0) {
        return PLOEG;
    } else if (strcmp(str, "CONSENSUS") == 0) {
        return CONSENSUS;
    } else if (strcmp(str, "FLATBED") == 0) {
        return FLATBED;
    } else if (strcmp(str, "DRIVER") == 0) {
        return DRIVER;
    } else {
        throw cRuntimeError("Invalid controller selected");
    }
}

} // namespace plexe
