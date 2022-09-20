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

package org.car2x.plexe.apps;

import org.car2x.plexe.apps.BaseApp;

simple DismantlePlatooningApp like BaseApp {

parameters:

    // implementation of the dismantle maneuver
    string dismantle;
    
	string joinManeuver;
	
	string mergeManeuver;
	
	int securityDistance;

    int headerLength @unit("bit") = default(0 bit);
    @display("i=block/app2");
    @class(plexe::DismantlePlatooningApp);

gates:
    input lowerLayerIn;
    output lowerLayerOut;
    input lowerControlIn;
    output lowerControlOut;
}