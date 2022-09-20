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

}

bool Dismantle::handleWarnDismantle(const WarnDismantle* msg)
{
}

bool Dismantle::handleWarnDismantleAck(const WarnDismantleAck* msg)
{
}

void Dismantle::handleAbort()
{
}

void Dismantle::abortManeuver()
{
}

void Dismantle::onFailedTransmissionAttempt(const ManeuverMessage* mm)
{
    throw cRuntimeError("Impossible to send this packet: %s. Maximum number of unicast retries reached", mm->getName());
}

void Dismantle::onManeuverMessage(const ManeuverMessage* mm)
{
}

void Dismantle::onPlatoonBeacon(const PlatooningBeacon* pb)
{

}

bool Dismantle::initializeDismantleManeuver()
{

}


bool Dismantle::processWarnDismantle(const WarnDismantle* msg)
{

}

bool Dismantle::processWarnDismantleAck(const WarnDismantleAck* msg)
{

}

} // namespace plexe
