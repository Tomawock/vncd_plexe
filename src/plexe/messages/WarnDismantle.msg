//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

cplusplus{{
#include "ManeuverMessage_m.h"
}};

packet ManeuverMessage;

// Request to a platoon to start the dismantle.
// Is sent from a leader of platoon to all platoon members.
packet WarnDismantle extends ManeuverMessage {
    // the distance in meters of the safe distance that everyone need to reach
    int dismantleDistance;
}