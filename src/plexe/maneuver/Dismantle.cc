#include "Dismantle.h"
#include "plexe/maneuver/Maneuver.h"
#include "plexe/apps/DismantlePlatooningApp.h"

#include "plexe/messages/WarnDismantle_m.h"
#include "plexe/messages/WarnDismantleAck_m.h"


namespace plexe {

Dismantle::Dismantle(GeneralPlatooningApp* app, int securityDistance)
    : Maneuver(app)
    , dismantleManeuverState(DismantleManeuverState::IDLE)
    , securityDistance(securityDistance)
{
}

void Dismantle::startManeuver(const void* parameters)
{
    if (initializeDismantleManeuver())
    {
        resetReceivedAck();
        // send Dismantle request to all followers
        sendDismantleRequest(positionHelper->getId(), positionHelper->getExternalId(), positionHelper->getPlatoonId());
    }

}

void Dismantle::resetReceivedAck() {
    for (int i : positionHelper->getPlatoonFormation()) {
        if (i != positionHelper->getLeaderId()) {
            receivedAck[i] = false;
        }
    }
}

bool Dismantle::initializeDismantleManeuver()
{
    if (dismantleManeuverState == DismantleManeuverState::IDLE && app->getPlatoonRole() == PlatoonRole::LEADER) {
        if (app->isInManeuver())
        {
            EV_INFO << positionHelper->getId() << " cannot begin the maneuver because already involved in another one\n";
            return false;
        }
        app->setInManeuver(true);

        // after successful initialization we are going to send a request and wait for a reply
        dismantleManeuverState = DismantleManeuverState::WAIT_REPLY;

        static_cast<DismantlePlatooningApp*>(app)->sendTimeoutMsg();

        return true;
    }
    else {
        return false;
    }
}

void Dismantle::sendDismantleRequest(int leaderId, std::string externalId, int platoonId)
{
    for (int i: positionHelper->getPlatoonFormation()) {
        if (i != leaderId){
            EV_INFO << positionHelper->getId() << " sending dismantleRequest to " << i << "\n";
            WarnDismantle* msg = new WarnDismantle("WarnDismantle"); // send dismantle distance to reach
            msg->setSafeDistance(100);
            app->fillManeuverMessage(msg, leaderId, externalId, platoonId, i);
            app->sendUnicast(msg, i);
        }
    }
}

void Dismantle::handleWarnDismantle(const WarnDismantle* msg)
{
    if (processWarnDismantle(msg)) {
        // send response to the leader
        EV_INFO << positionHelper->getId() << " sending DismantleAck to the leader (" << msg->getVehicleId() << ")\n";
        WarnDismantleAck* response = new WarnDismantleAck("WarnDismantleAck");
        response->setResponse(true);
        app->fillManeuverMessage(response, positionHelper->getId(), positionHelper->getExternalId(), positionHelper->getPlatoonId(), msg->getVehicleId());
        app->sendUnicast(response, msg->getVehicleId());
    }
    else {
        abortManeuver();
    }
}

bool Dismantle::processWarnDismantle(const WarnDismantle* msg)
{
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()) return false;

    if (app->getPlatoonRole() != PlatoonRole::FOLLOWER) return false;

    if (dismantleManeuverState != DismantleManeuverState::IDLE || app->isInManeuver()) return false;

    app->setInManeuver(true);

    dismantleManeuverState = DismantleManeuverState::PREPARE_DISMANTLE;
    return true;
}


void Dismantle::handleWarnDismantleAck(const WarnDismantleAck* msg)
{
    if (processWarnDismantleAck(msg)) {

        EV_INFO << "HANDLE WARN DISMANTLE ACK" << endl;
//        plexeTraciVehicle->setFixedLane(nextDestination, false);
//        positionHelper->setPlatoonLane(nextDestination);
          // send response to followers
//        for (int i: positionHelper->getPlatoonFormation())
//        {
//            if (i != positionHelper->getLeaderId())
//            {
//                LOG << "Leader " << positionHelper->getId() << " sending startSignal to the follower with id " << i << "\n";
//                StartSignal* response = new StartSignal("StartSignal");
//                app->fillManeuverMessage(response, positionHelper->getId(), positionHelper->getExternalId(), positionHelper->getPlatoonId(), i);
//                app->sendUnicast(response, i);
//            }
//        }
//        laneChangeManeuverState = LaneChangeManeuverState::WAIT_ALL_CHANGED;
//
//        static_cast<LaneChangePlatooningApp*>(app)->sendTimeoutMsg();
    }
}

bool Dismantle::processWarnDismantleAck(const WarnDismantleAck* msg)
{
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()) return false;

    if (app->getPlatoonRole() != PlatoonRole::LEADER) return false;

    if (dismantleManeuverState != DismantleManeuverState::WAIT_REPLY || !app->isInManeuver()) return false;

    receivedAck[msg->getVehicleId()] = true;

    EV_INFO << "ACK RECIVED FROM_" << msg->getVehicleId() << endl;

    for (auto const& x : receivedAck)
    {
        if(!x.second) return false;
    }
    EV_INFO << "ALL ACK RECIVED" << endl;

    static_cast<DismantlePlatooningApp*>(app)->resetTimeoutMsg();
    dismantleManeuverState = DismantleManeuverState::SAFE_DISTANCE;
    resetReceivedAck();
    return true;
}

void Dismantle::handleAbort()
{
}

void Dismantle::abortManeuver()
{
    // TODO
}

void Dismantle::onFailedTransmissionAttempt(const ManeuverMessage* mm)
{
    throw cRuntimeError("Impossible to send this packet: %s. Maximum number of unicast retries reached", mm->getName());
}

void Dismantle::onManeuverMessage(const ManeuverMessage* mm)
{
    if (const WarnDismantle* msg = dynamic_cast<const WarnDismantle*>(mm)) {
        handleWarnDismantle(msg);
    } else if (const WarnDismantleAck* msg = dynamic_cast<const WarnDismantleAck*>(mm)) {
        handleWarnDismantleAck(msg);
    }
//    } else if (const StartSignal* msg = dynamic_cast<const StartSignal*>(mm)) {
//        handleStartSignal(msg);
//    } else if (const LaneChanged* msg = dynamic_cast<const LaneChanged*>(mm)) {
//        handleLaneChanged(msg);
//    } else if (const LaneChangeClose* msg = dynamic_cast<const LaneChangeClose*>(mm)) {
//        handleLaneChangeClose(msg);
//    } else if (const Again* msg = dynamic_cast<const Again*>(mm)) {
//        handleAgain(msg);
//    } else if (const Abort* msg = dynamic_cast<const Abort*>(mm)) {
//        handleAbort();
//    }

}

void Dismantle::onPlatoonBeacon(const PlatooningBeacon* pb)
{

}

} // namespace plexe
