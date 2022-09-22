//
// Copyright (C) 2012-2021 Michele Segata <segata@ccs-labs.org>
// Copyright (C) 2018-2021 Julian Heinovski <julian.heinovski@ccs-labs.org>
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

#include "plexe/apps/GeneralPlatooningApp.h"
#include "plexe/protocols/BaseProtocol.h"
#include "plexe/apps/DismantlePlatooningApp.h"
#include "veins/modules/mac/ieee80211p/Mac1609_4.h"


using namespace veins;

namespace plexe {

Define_Module(DismantlePlatooningApp);

void DismantlePlatooningApp::initialize(int stage)
{
    GeneralPlatooningApp::initialize(stage);

    if (stage == 1) {
        std::string dismantleManeuverName = par("dismantle").stdstringValue();

        if (dismantleManeuverName == "dismantle"){
            dismantleManeuver = new Dismantle(this,
                    par("dismantleDistance").intValue(),
                    getEnum(par("dismantleController").stringValue()),
                    par("decelleration").doubleValue());
        }else {
            throw new cRuntimeError("Invalid Dismantle maneuver implementation chosen");
        }
    }
}

void DismantlePlatooningApp::onPlatoonBeacon(const PlatooningBeacon* pb)
{
    dismantleManeuver->onPlatoonBeacon(pb);
    // maintain platoon
    BaseApp::onPlatoonBeacon(pb);
}

void DismantlePlatooningApp::onManeuverMessage(ManeuverMessage* mm)
{
    dismantleManeuver->onManeuverMessage(mm);
    delete mm;
}


void DismantlePlatooningApp::startDismantleManeuver(int platoonId, int leaderId)
{
    ASSERT(getPlatoonRole() == PlatoonRole::NONE);
    ASSERT(!isInManeuver());
    if (platoonId != 0){
        dismantleManeuver->startManeuver(nullptr);
    }else{
        dismantleManeuver->startManeuver(nullptr);
    }

}



void DismantlePlatooningApp::receiveSignal(cComponent* src, simsignal_t id, cObject* value, cObject* details)
{
    if (id == Mac1609_4::sigRetriesExceeded) {
        BaseFrame1609_4* frame = check_and_cast<BaseFrame1609_4*>(value);
        ManeuverMessage* mm = check_and_cast<ManeuverMessage*>(frame->getEncapsulatedPacket());
        if (frame) {
            dismantleManeuver->onFailedTransmissionAttempt(mm);
        }
    }
}

void DismantlePlatooningApp::sendTimeoutMsg()
{
    timeoutMsg = new cMessage("TimeoutMsg");
    take(timeoutMsg);
    scheduleAt(simTime() + SimTime(1), timeoutMsg);
}

void DismantlePlatooningApp::resetTimeoutMsg()
{
    cancelAndDelete(timeoutMsg);
}

void DismantlePlatooningApp::sendTimeoutAccelleration()
{
    accellerationMsg = new cMessage("AccellerationMsg");
    take(accellerationMsg);
    scheduleAt(simTime() + SimTime(0.1), accellerationMsg);
}


void DismantlePlatooningApp::handleSelfMsg(cMessage* msg)
{
    if (dismantleManeuver && dismantleManeuver->handleSelfMsg(msg)) return;
    BaseApp::handleSelfMsg(msg);
}

void DismantlePlatooningApp::resetTimeoutAccelleration()
{
    cancelAndDelete(accellerationMsg);
}

DismantlePlatooningApp::~DismantlePlatooningApp()
{
    delete dismantleManeuver;
}
ACTIVE_CONTROLLER DismantlePlatooningApp::getEnum(const char* str)
{
    if (strcmp(str, "ACC") == 0) {
        return ACTIVE_CONTROLLER::ACC;
    } else if (strcmp(str, "CACC") == 0) {
        return ACTIVE_CONTROLLER::CACC;
    } else if (strcmp(str, "PLOEG") == 0) {
        return ACTIVE_CONTROLLER::PLOEG;
    } else if (strcmp(str, "CONSENSUS") == 0) {
        return ACTIVE_CONTROLLER::CONSENSUS;
    } else if (strcmp(str, "FLATBED") == 0) {
        return ACTIVE_CONTROLLER::FLATBED;
    } else if (strcmp(str, "DRIVER") == 0) {
        return ACTIVE_CONTROLLER::DRIVER;
    } else if (strcmp(str, "FAKED_CACC") == 0) {
        return ACTIVE_CONTROLLER::FAKED_CACC;
    } else {
        throw cRuntimeError("Invalid controller selected");
    }
}

} // namespace plexe
