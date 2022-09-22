#include "Dismantle.h"
#include "plexe/maneuver/Maneuver.h"
#include "plexe/apps/DismantlePlatooningApp.h"

#include "plexe/messages/WarnDismantle_m.h"
#include "plexe/messages/WarnDismantleAck_m.h"
#include "plexe/messages/MoveToSafeDistance_m.h"
#include "plexe/messages/SafeDistanceReached_m.h"
#include "plexe/messages/SafeDistanceAccelleration_m.h"
#include "plexe/messages/DismantleCommand_m.h"
#include "plexe/messages/DismantleAck_m.h"

// TODO NOT IMPLEMNTED ALL THE ABORT and all the case in whitch they are needed

namespace plexe {

Dismantle::Dismantle(GeneralPlatooningApp* app, int dismantleDistance, ACTIVE_CONTROLLER dismantleController,double decelleration)
    : Maneuver(app)
    , dismantleManeuverState(DismantleManeuverState::IDLE)
    , dismantleDistance(dismantleDistance)
    , dismantleController(dismantleController)
    , decelleration(decelleration)
{
}

bool Dismantle::handleSelfMsg(cMessage* msg)
{
    std::string title = msg->getName();
    if (title.compare("TimeoutMsg") == 0) {
        abortManeuver();
    }
    else if(title.compare("AccellerationMsg") == 0){
        handleSafeDistanceAccelleration();
    }

    return true;
}

void Dismantle::destroyManeuver()
{
    plexeTraciVehicle->setActiveController(dismantleController);
    plexeTraciVehicle->setFixedLane(-1);
}

void Dismantle::startManeuver(const void* parameters)
{
    destroyManeuver();
//    if (initializeDismantleManeuver())
//    {
//        resetReceivedAck();
//        // send Dismantle request to all followers
//        sendDismantleRequest(positionHelper->getId(), positionHelper->getExternalId(), positionHelper->getPlatoonId());
//    }
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
            EV_INFO << positionHelper->getId() << " sending WarnDismantleRequest to " << i << "\n";
            WarnDismantle* msg = new WarnDismantle("WarnDismantle"); // send dismantle distance to reach
            msg->setDismantleDistance(dismantleDistance);
            app->fillManeuverMessage(msg, leaderId, externalId, platoonId, i);
            app->sendUnicast(msg, i);
        }
    }
}

void Dismantle::handleWarnDismantle(const WarnDismantle* msg)
{
    if (processWarnDismantle(msg)) {
        // save the distance that needs to be reached if the manover starts
        dismantleDistance = msg->getDismantleDistance();
        EV_INFO << "SAFE DISTANCE VALUE:\t" << dismantleDistance << endl;
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
        // send response to followers
        for (int i: positionHelper->getPlatoonFormation())
        {
            if (i != positionHelper->getLeaderId())
            {
                EV_INFO << "Leader " << positionHelper->getId() << " sending moveToSafeDistance to the follower with id " << i << "\n";
                MoveToSafeDistance* msg = new MoveToSafeDistance("MoveToSafeDistance");
                msg->setMoveToSafeDistanceAccelleration(decelleration);
                app->fillManeuverMessage(msg, positionHelper->getId(), positionHelper->getExternalId(), positionHelper->getPlatoonId(), i);
                app->sendUnicast(msg, i);
            }
        }
        dismantleManeuverState = DismantleManeuverState::WAIT_SAFE_DISTANCE_REACHED;

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

    EV_INFO << "ALL WarnDismantleAck RECIVED BY THE LEADER" << endl;
    static_cast<DismantlePlatooningApp*>(app)->resetTimeoutMsg();
    dismantleManeuverState = DismantleManeuverState::SAFE_DISTANCE;
    resetReceivedAck();
    return true;
}

void Dismantle::handleMoveToSafeDistance(const MoveToSafeDistance* msg)
{
    EV_INFO << "OK_HANDLE" <<endl;
    if (processMoveToSafeDistance(msg)) {
        double speed = -1.0;
        double acceleration = -1.0;
        double distance = -1.0;
        double controllerAcceleration = -1.0;
        double positionX = -1.0;
        double positionY = -1.0;
        double time = -1.0;

        plexeTraciVehicle->getVehicleData(speed,acceleration, controllerAcceleration, positionX, positionY, time);
        plexeTraciVehicle->getRadarMeasurements(distance,speed);

        int platoonPosition = positionHelper->getPosition();
        // compute decelleration
        // farla variare ad ogni 0.1 per essere piu precisi
        double actual_accelleration = (distance - dismantleDistance) / (msg->getMoveToSafeDistanceAccelleration() * msg->getMoveToSafeDistanceAccelleration());
//        msg->getMoveToSafeDistanceAccelleration()*platoonPosition
        plexeTraciVehicle->setFixedAcceleration(1,actual_accelleration * platoonPosition);
        static_cast<DismantlePlatooningApp*>(app)->sendTimeoutAccelleration();  // send self message in order to arrive to a safe distance
    }
    else {
        abortManeuver();
    }
}

bool Dismantle::processMoveToSafeDistance(const MoveToSafeDistance* msg)
{

    if (msg->getPlatoonId() != positionHelper->getPlatoonId()) return false;

    if (app->getPlatoonRole() != PlatoonRole::FOLLOWER) return false;

    if (dismantleManeuverState != DismantleManeuverState::PREPARE_DISMANTLE || !app->isInManeuver()) return false;

    dismantleManeuverState = DismantleManeuverState::SAFE_DISTANCE;

    return true;
}

void Dismantle::handleSafeDistanceAccelleration(){

    if(!processSafeDistanceAccelleration()){
        // send to leader position reached
        EV_INFO << "Sending SafeDistanceReached to the Leader: " << positionHelper->getLeaderId() << "\n";
        SafeDistanceReached* msg = new SafeDistanceReached("SafeDistanceReached");
        msg->setSafeDistanceReached(true);
        app->fillManeuverMessage(msg, positionHelper->getId(), positionHelper->getExternalId(), positionHelper->getPlatoonId(), positionHelper->getLeaderId());
        app->sendUnicast(msg, positionHelper->getLeaderId());
    }
}

bool Dismantle::processSafeDistanceAccelleration()
{
    double speed = -1.0;
    double acceleration = -1.0;
    double relativeSpeed = -1.0;
    double distance = -1.0;
    double controllerAcceleration = -1.0;
    double positionX = -1.0;
    double positionY = -1.0;
    double time = -1.0;

    plexeTraciVehicle->getVehicleData(speed,acceleration, controllerAcceleration, positionX, positionY, time);
    EV_INFO << "speed\t" << speed << endl;
    EV_INFO << "acceleration\t" << acceleration << endl;

    plexeTraciVehicle->getRadarMeasurements(distance,relativeSpeed);

    if(distance > dismantleDistance)
    {
        EV_INFO <<"SAFE DISTANCE REACHED\t"<< distance << endl;
        plexeTraciVehicle->setFixedAcceleration(0,0.0);
        static_cast<DismantlePlatooningApp*>(app)->resetTimeoutAccelleration();
        dismantleManeuverState = DismantleManeuverState::DISMANTLE;
        return false;
    } else
    {
        EV_INFO <<"SAFE DISTANCE NOT REACHED\t"<< distance << " by\t" << positionHelper->getId() << endl;
        static_cast<DismantlePlatooningApp*>(app)->sendTimeoutAccelleration();
        return true;
    }
    return true;
}

void Dismantle::handleSafeDistanceReached(const SafeDistanceReached* msg)
{
    if (processSafeDistanceReached(msg)) {
        // send dismantle
        for (int i: positionHelper->getPlatoonFormation()) {
            if (i != positionHelper->getLeaderId()){
                EV_INFO << positionHelper->getId() << " sending dismantleCommand to " << i << "\n";
                DismantleCommand* msg = new DismantleCommand("DismantleCommand"); // send dismantle distance to reach
                msg->setDismantle(true);
                app->fillManeuverMessage(msg, positionHelper->getLeaderId(), positionHelper->getExternalId(), positionHelper->getPlatoonId(), i);
                app->sendUnicast(msg, i);
            }
        }
    }
    else {
        abortManeuver();
    }
}

bool Dismantle::processSafeDistanceReached(const SafeDistanceReached* msg)
{
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()) return false;

    if (app->getPlatoonRole() != PlatoonRole::LEADER) return false;

    if (dismantleManeuverState != DismantleManeuverState::WAIT_SAFE_DISTANCE_REACHED || !app->isInManeuver()) return false;

    receivedAck[msg->getVehicleId()] = true;

    EV_INFO << "SAFE DISTANCE REACHED BY_" << msg->getVehicleId() << endl;

    for (auto const& x : receivedAck)
    {
        if(!x.second) return false;
    }

    EV_INFO << "ALL SafeDistanceReached RECIVED BY THE LEADER" << endl;
    dismantleManeuverState = DismantleManeuverState::WAIT_DISMANTLE;

    static_cast<DismantlePlatooningApp*>(app)->sendTimeoutMsg();

    resetReceivedAck();
    return true;
}

void Dismantle::handleDismantleCommand(const DismantleCommand* msg)
{
    if (processDismantleCommand(msg)) {
        // send ack to leader
        EV_INFO << "Sending DismantleAck to the Leader: " << positionHelper->getLeaderId() << "\n";
        DismantleAck* msg = new DismantleAck("DismantleAck");
        msg->setResponse(true);
        app->fillManeuverMessage(msg, positionHelper->getId(), positionHelper->getExternalId(), positionHelper->getPlatoonId(), positionHelper->getLeaderId());
        app->sendUnicast(msg, positionHelper->getLeaderId());
    }
    else {
        abortManeuver();
    }
}

bool Dismantle::processDismantleCommand(const DismantleCommand* msg)
{
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()) return false;

    if (app->getPlatoonRole() != PlatoonRole::FOLLOWER) return false;

    if (dismantleManeuverState != DismantleManeuverState::DISMANTLE || !app->isInManeuver()) return false;

    EV_INFO << "DISMANTLE COMMAND RECIVED" << endl;

    plexeTraciVehicle->setActiveController(dismantleController);
    plexeTraciVehicle->setFixedLane(-1);
//    plexeTraciVehicle->setCruiseControlDesiredSpeed(200 / 3.6);

    EV_INFO << "ACTIVE CONTROLLER "<<plexeTraciVehicle->getActiveController() << endl;

    dismantleManeuverState = DismantleManeuverState::ACC_IDM;

    return true;
}

void Dismantle::handleDismantleAck(const DismantleAck* msg)
{
    if (processDismantleAck(msg)) {
        EV_INFO << "INITIAL SIZE PLATOON\t" << positionHelper->getPlatoonSize() << endl;
        // remove all elements from platoon
        std::vector<int> newFormation{};
        positionHelper->setPlatoonFormation(newFormation);
        EV_INFO << "FINAL SIZE PLATOON\t" << positionHelper->getPlatoonSize() << endl;
        plexeTraciVehicle->setActiveController(dismantleController);
        plexeTraciVehicle->setFixedLane(-1);
    }
    else {
        abortManeuver();
    }
}

bool Dismantle::processDismantleAck(const DismantleAck* msg)
{
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()) return false;

    if (app->getPlatoonRole() != PlatoonRole::LEADER) return false;

    if (dismantleManeuverState != DismantleManeuverState::WAIT_DISMANTLE || !app->isInManeuver()) return false;

    receivedAck[msg->getVehicleId()] = true;

    EV_INFO << "DISMANTLE ACK RECIVED BY_" << msg->getVehicleId() << endl;

    for (auto const& x : receivedAck)
    {
        if(!x.second) return false;
    }

    EV_INFO << "ALL DismantleAck RECIVED BY THE LEADER" << endl;

    static_cast<DismantlePlatooningApp*>(app)->resetTimeoutMsg();

    resetReceivedAck();

    dismantleManeuverState = DismantleManeuverState::ACC_IDM;

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
    } else if (const MoveToSafeDistance* msg = dynamic_cast<const MoveToSafeDistance*>(mm)) {
        handleMoveToSafeDistance(msg);
    } else if (const SafeDistanceReached* msg = dynamic_cast<const SafeDistanceReached*>(mm)) {
        handleSafeDistanceReached(msg);
    } else if (const DismantleCommand* msg = dynamic_cast<const DismantleCommand*>(mm)) {
        handleDismantleCommand(msg);
    } else if (const DismantleAck* msg = dynamic_cast<const DismantleAck*>(mm)) {
        handleDismantleAck(msg);
    }
}

void Dismantle::onPlatoonBeacon(const PlatooningBeacon* pb)
{

}

} // namespace plexe
