#ifndef DISMANTLE_H_
#define DISMANTLE_H_


#include <algorithm>

#include "plexe/maneuver/Maneuver.h"

#include "plexe/messages/WarnDismantle_m.h"
#include "plexe/messages/WarnDismantleAck_m.h"
//#include "plexe/messages/MoveToSafeDistance_m.h"
//#include "plexe/messages/MoveToSafeDistanceReached_m.h"
//#include "plexe/messages/Disamntle_m.h"
//#include "plexe/messages/DisamntleAck_m.h"


using namespace veins;

namespace plexe {

class Dismantle : public Maneuver {

public:
    Dismantle(GeneralPlatooningApp* app, int securityDistance);
    ~Dismantle(){};

    virtual void startManeuver(const void* parameters) override;

    /** initializes the handling of a WarnDisamntle */
    bool handleWarnDismantle(const WarnDismantle* msg);

    /** initializes the handling of a WarnDisamntle */
    bool handleWarnDismantleAck(const WarnDismantleAck* msg);

    void handleAbort();

    virtual void abortManeuver() override;
    virtual void onFailedTransmissionAttempt(const ManeuverMessage* mm) override;
    virtual void onManeuverMessage(const ManeuverMessage* mm) override;
    virtual void onPlatoonBeacon(const PlatooningBeacon* pb) override;
protected:
    /** Possible states a vehicle can be in during a Disamntle maneuver */
    enum class DismantleManeuverState {
        IDLE,                           ///< The maneuver did not start
        SAFE_DISTANCE,                  ///< The
        ACC_IDM,                        ///< The maneuver did not start
        // LEADER
        WAIT_REPLY,                     ///< The maneuver is running
        WAIT_SAFE_DISTANCE_REACHED,     ///< The maneuver did not start
        WAIT_DISMANTLE,                 ///< The maneuver did not start
        // FOLLOWER
        PREPARE_DISMANTLE,              ///< The maneuver did not start
        DISMANTLE,                      ///< The follower evalueate if is possible to change and if possible send an ack
    };

    /** the current state in the Disamntle maneuver */
    DismantleManeuverState dismantleManeuverState;

    /** initializes a Disamntle maneuver, setting up required data */
    bool initializeDismantleManeuver();

    /** initializes the handling of a WarnDisamntle */
    bool processWarnDismantle(const WarnDismantle* msg);

    /** initializes the handling of a WarnDisamntle */
    bool processWarnDismantleAck(const WarnDismantleAck* msg);


private:
//    void sendDismantleRequest(int leaderId, std::string externalId, int platoonId);
    int securityDistance;
};

} // namespace plexe

#endif
