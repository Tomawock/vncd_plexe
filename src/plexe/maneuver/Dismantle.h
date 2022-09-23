#ifndef DISMANTLE_H_
#define DISMANTLE_H_


#include <algorithm>

#include "plexe/maneuver/Maneuver.h"

#include "plexe/messages/WarnDismantle_m.h"
#include "plexe/messages/WarnDismantleAck_m.h"
#include "plexe/messages/MoveToSafeDistance_m.h"
#include "plexe/messages/SafeDistanceReached_m.h"
#include "plexe/messages/SafeDistanceAccelleration_m.h"
#include "plexe/messages/DismantleCommand_m.h"
#include "plexe/messages/DismantleAck_m.h"

#include "plexe/messages/Destroy_m.h"


using namespace veins;

namespace plexe {

class Dismantle : public Maneuver {

public:
    Dismantle(GeneralPlatooningApp* app, int securityDistance,ACTIVE_CONTROLLER dismantleController, double decelleration );
    ~Dismantle(){};

    virtual void startManeuver(const void* parameters) override;

    void destroyManeuver();

    void handleDestroy(const Destroy* msg);

    /** initializes the handling of a WarnDisamntle */
    void handleWarnDismantle(const WarnDismantle* msg);

    /** initializes the handling of a WarnDisamntle */
    void handleWarnDismantleAck(const WarnDismantleAck* msg);

    void handleMoveToSafeDistance(const MoveToSafeDistance* msg);

    void handleSafeDistanceAccelleration();

    void handleSafeDistanceReached(const SafeDistanceReached* msg);

    void handleDismantleCommand(const DismantleCommand* msg);

    void handleDismantleAck(const DismantleAck* msg);

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

    bool processSafeDistanceAccelleration();

    /** initializes the handling of a WarnDisamntle */
    bool processWarnDismantleAck(const WarnDismantleAck* msg);

    bool processMoveToSafeDistance(const MoveToSafeDistance* msg);

    bool processSafeDistanceReached(const SafeDistanceReached* msg);

    bool processDismantleCommand(const DismantleCommand* msg);

    bool processDismantleAck(const DismantleAck* msg);

    virtual bool handleSelfMsg(cMessage* msg) override;
private:
    void resetReceivedAck();
    void sendDismantleRequest(int leaderId, std::string externalId, int platoonId);

    ACTIVE_CONTROLLER dismantleController;
    int dismantleDistance;
    double decelleration;
    std::map<int, bool> receivedAck;
};

} // namespace plexe

#endif
