#ifndef _default_vars_h_
#define _default_vars_h_
// gbzadmin
// GTKmm bzadmin
// Copyright (c) 2005 - 2012 Michael Sheppard
//  
// Code based on BZFlag-2.0.x
// Portions Copyright (c) 1993 - 2005 Tim Riker
// 
//  =====GPL=============================================================
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; version 2 dated June, 1991.
// 
//  This program is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program;  if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave., Cambridge, MA 02139, USA.
//  =====================================================================

// 2.4.0 default variable values
static const char* defaults[][256] = {
	{"_HTTPIndexResourceDir", ""},
	{"_agilityAdVel", "2.25"},
	{"_agilityTimeWindow", "1"},
	{"_agilityVelDelta", "0.3"},
	{"_ambientLight", "none"},
	{"_angleTolerance", "0.05"},
	{"_angularAd", "1.5"},
	{"_avenueSize", "2.0*_boxBase"},
	{"_baseSize", "60"},
	{"_boxBase", "30"},
	{"_boxHeight", "6.0*_muzzleHeight"},
	{"_burrowAngularAd", "0.55"},
	{"_burrowDepth", "-1.32"},
	{"_burrowSpeedAd", "0.8"},
	{"_coldetDepth", "6"},
	{"_coldetElements", "4"},
	{"_countdownResumeDelay", "5"},
	{"_cullDepth", "6"},
	{"_cullDist", "fog"},
	{"_cullElements", "16"},
	{"_cullOccluders", "0"},
	{"_disableBots", "0"},
	{"_disableHeightChecks", "0"},
	{"_disableSpeedChecks", "0"},
	{"_drawCelestial", "1"},
	{"_drawClouds", "1"},
	{"_drawGround", "1"},
	{"_drawGroundLights", "1"},
	{"_drawMountains", "1"},
	{"_drawSky", "1"},
	{"_enableDistanceCheck", "0"},
	{"_endShotDetection", "5"},
	{"_explodeTime", "5"},
	{"_flagAltitude", "11"},
	{"_flagEffectTime", "0.64"},
	{"_flagHeight", "10"},
	{"_flagPoleSize", "0.8"},
	{"_flagPoleWidth", "0.025"},
	{"_flagRadius", "2.5"},
	{"_fogColor", "0.25 0.25 0.25"},
	{"_fogDensity", "0.001"},
	{"_fogEnd", "_worldSize"},
	{"_fogMode", "none"},
	{"_fogNoSky", "0"},
	{"_fogStart", "0.5*_worldSize"},
	{"_forbidHunting", "0"},
	{"_forbidIdentify", "0"},
	{"_forbidMarkers", "0"},
	{"_friction", "0"},
	{"_gmActivationTime", "0.5"},
	{"_gmAdLife", "0.95"},
	{"_gmSize", "1.5"},
	{"_gmTurnAngle", "0.628319"},
	{"_gravity", "-9.8"},
	{"_handicapAngAd", "1.5"},
	{"_handicapScoreDiff", "50"},
	{"_handicapShotAd", "1.75"},
	{"_handicapVelAd", "2"},
	{"_hideFlagsOnRadar", "0"},
	{"_hideTeamFlagsOnRadar", "0"},
	{"_identifyRange", "50"},
	{"_jumpVelocity", "19"},
	{"_lRAdRate", "0.5"},
	{"_laserAdLife", "0.1"},
	{"_laserAdRate", "0.5"},
	{"_laserAdVel", "1000"},
	{"_latitude", "37.5"},
	{"_lockOnAngle", "0.15"},
	{"_longitude", "122"},
	{"_mGunAdLife", "1 / _mGunAdRate"},
	{"_mGunAdRate", "10"},
	{"_mGunAdVel", "1.5"},
	{"_maxBumpHeight", "0.33"},
	{"_maxFlagGrabs", "4"},
	{"_maxLOD", "32767"},
	{"_mirror", "none"},
	{"_momentumAngAcc", "1"},
	{"_momentumFriction", "0"},
	{"_momentumLinAcc", "1"},
	{"_muzzleFront", "_tankRadius + 0.1"},
	{"_muzzleHeight", "1.57"},
	{"_noClimb", "1"},
	{"_noShadows", "0"},
	{"_noSmallPackets", "0"},
	{"_notRespondingTime", "5"},
	{"_obeseFactor", "2.5"},
	{"_pauseDropTime", "15"},
	{"_positionTolerance", "0.09"},
	{"_pyrBase", "4.0*_tankHeight"},
	{"_pyrHeight", "5.0*_tankHeight"},
	{"_rFireAdLife", "1 / _rFireAdRate"},
	{"_rFireAdRate", "2"},
	{"_rFireAdVel", "1.5"},
	{"_radarLimit", "_worldSize"},
	{"_rainBaseColor", "none"},
	{"_rainDensity", "none"},
	{"_rainEndZ", "none"},
	{"_rainMaxPuddleTime", "none"},
	{"_rainPuddleColor", "none"},
	{"_rainPuddleSpeed", "none"},
	{"_rainPuddleTexture", "none"},
	{"_rainRoofs", "1"},
	{"_rainSpeed", "none"},
	{"_rainSpeedMod", "none"},
	{"_rainSpins", "none"},
	{"_rainSpread", "none"},
	{"_rainStartZ", "none"},
	{"_rainTexture", "none"},
	{"_rainTopColor", "none"},
	{"_rainType", "none"},
	{"_rejoinTime", "_explodeTime"},
	{"_reloadTime", "_shotRange / _shotSpeed"},
	{"_shieldFlight", "2.7"},
	{"_shockAdLife", "0.2"},
	{"_shockInRadius", "_tankLength"},
	{"_shockOutRadius", "60"},
	{"_shotRadius", "0.5"},
	{"_shotRange", "350"},
	{"_shotSpeed", "100"},
	{"_shotTailLength", "4"},
	{"_shotsKeepVerticalVelocity", "0"},
	{"_skyColor", "white"},
	{"_spawnMaxCompTime", "0.01"},
	{"_spawnSafeRadMod", "20"},
	{"_spawnSafeSRMod", "3"},
	{"_spawnSafeSWMod", "1.5"},
	{"_speedChecksLogOnly", "0"},
	{"_squishFactor", "1"},
	{"_squishTime", "1"},
	{"_srRadiusMult", "2"},
	{"_syncLocation", "0"},
	{"_syncTime", "-1"},
	{"_tankAngVel", "0.785398"},
	{"_tankExplosionSize", "3.5 * _tankLength"},
	{"_tankHeight", "2.05"},
	{"_tankLength", "6"},
	{"_tankRadius", "0.72 * _tankLength"},
	{"_tankSpeed", "25"},
	{"_tankWidth", "2.8"},
	{"_targetingAngle", "0.3"},
	{"_targetingDistance", "300"},
	{"_teleportBreadth", "4.48"},
	{"_teleportHeight", "10.08"},
	{"_teleportTime", "1"},
	{"_teleportWidth", "1.12"},
	{"_thiefAdLife", "0.05"},
	{"_thiefAdRate", "12"},
	{"_thiefAdShotVel", "8"},
	{"_thiefDropTime", "0.5 * _reloadTime"},
	{"_thiefTinyFactor", "0.5"},
	{"_thiefVelAd", "1.67"},
	{"_tinyFactor", "0.4"},
	{"_trackFade", "3"},
	{"_updateThrottleRate", "30"},
	{"_useLineRain", "none"},
	{"_useRainBillboards", "none"},
	{"_useRainPuddles", "none"},
	{"_velocityAd", "1.5"},
	{"_wallHeight", "3.0*_tankHeight"},
	{"_weapons", "1"},
	{"_wideAngleAng", "1.745329"},
	{"_wingsGravity", "_gravity"},
	{"_wingsJumpCount", "1"},
	{"_wingsJumpVelocity", "_jumpVelocity"},
	{"_wingsSlideTime", "0"},
	{"_worldSize", "800"},
	{"poll", ""}
};

const int maxIdx = sizeof(defaults) / sizeof(defaults[0]);

class serverDBItems {
	typedef std::vector<Glib::ustring> Names;
	typedef std::vector<Glib::ustring> DefValues;
	Names name;
	DefValues defValue;

public:
	void init()
	{
		for (int k = 0; k < maxIdx; k++) {
			name.push_back(defaults[k][0]);
			defValue.push_back(defaults[k][1]);
		}
	}

	Glib::ustring find(Glib::ustring variable)
	{
		Glib::ustring var("");
		bool found = false;
		
		for (std::vector<Glib::ustring>::size_type k = 0; k != name.size(); k++) {
			if (name.at(k) == variable) {
				var = defValue.at(k);
				found = true;
				break;
			}
		}
		if (!found) {
			var = "UNKNOWN";
		}
		return var;
	}
};

#endif // _default_vars_h_

