
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

#ifndef DISMANTLEPLATOONINGAPP_H_
#define DISMANTLEPLATOONINGAPP_H_

#include <algorithm>
#include <memory>

#include "plexe/apps/BaseApp.h"
#include "plexe/apps/GeneralPlatooningApp.h"
#include "plexe/maneuver/Maneuver.h"
#include "plexe/maneuver/Dismantle.h"


#include "plexe/messages/ManeuverMessage_m.h"

#include "plexe/scenarios/BaseScenario.h"

#include "veins/modules/mobility/traci/TraCIConstants.h"
#include "veins/modules/utility/SignalManager.h"

namespace plexe {

class DismantlePlatooningApp : public GeneralPlatooningApp {

public:
    /** c'tor for DismantlePlatooningApp */
    DismantlePlatooningApp()
        : GeneralPlatooningApp()
        , dismantleManeuver(nullptr)
    {
    }

    /** d'tor for DismantlePlatooningApp */
    virtual ~DismantlePlatooningApp();

    virtual void startDismantleManeuver(int platoonId, int leaderId);

    /** override from GeneralPlatooningApp */
    virtual void initialize(int stage) override;

    void sendTimeoutMsg();

    void resetTimeoutMsg();

    void sendTimeoutAccelleration();

    void resetTimeoutAccelleration();

protected:
    /** used to receive the "retries exceeded" signal **/
    virtual void receiveSignal(cComponent* src, simsignal_t id, cObject* value, cObject* details) override;

    /**
     * Handles PlatoonBeacons
     *
     * @param PlatooningBeacon pb to handle
     */
    virtual void onPlatoonBeacon(const PlatooningBeacon* pb) override;

    /**
     * Handles ManeuverMessages
     *
     * @param ManeuverMessage mm to handle
     */
    virtual void onManeuverMessage(ManeuverMessage* mm);

    virtual void handleSelfMsg(cMessage* msg) override;


private:
    ACTIVE_CONTROLLER getEnum(const char* str);

    /** platoons change lane implementation */
    Maneuver* dismantleManeuver;

    // message used to schedule timeouts
    cMessage* timeoutMsg;

    // message used to schedule timeouts
    cMessage* accellerationMsg;
};

} // namespace plexe

#endif
