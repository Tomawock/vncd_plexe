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

package org.car2x.plexe.scenarios;

import org.car2x.plexe.scenarios.BBaseScenario;

simple DismantleManeuverScenario extends BBaseScenario
{
    parameters:
        @display("i=block/app2");
        @class(plexe::DismantleManeuverScenario);
        
        string leaderController;
        string followerController;
        string otherCarController;
        
        int numberOfCars;
        int numberOfCarsPerPlatoon;
        
        int meneuverTime;
        
        int desiredPlatoonSpeed;
        int desiredOtherCarSpeed;
}