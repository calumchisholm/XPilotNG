Summary: A multiuser gravity war game client
Name: xpilot
Version: 4.5.4
Release: x2
Packager: darel.cullen@bostream.nu
Url: http://xpilot.sf.net
Source0: %{name}-%{version}.tar.gz
License: GPL
Group: Amusements/Games
Vendor: XPilot
BuildRoot: /temp/%{name}-root

%description
XPilot is a multi-player 2D space game. Some features are borrowed 
from classics like the Atari coin-ups Asteroids and Gravitar, and
the home-computer games Thrust (Commodore 64) and Gravity Force
(Commodore Amiga), but XPilot has many new aspects too. 
Highlights include:

    * True client/server based game; optimal speed for every player.
    * Meta server with up to date information about servers hosting 
      games around the world.
    * 'Real physics'; particles of explosions and sparks from your 
      engines all affect you if you're hit by them. This makes it 
      possible to kill someone by blowing them into a wall with engine 
      thrust or shock waves from explosions.
    * Specialized editors for editing ship-shapes and maps.
    * Game objective and gameplay adjustable through a number of 
      options, specified on the commandline, in special option files, 
      or in the map files. Examples of modes of the game:
    * FPS values are set by the server
    * textures and polygon maps

    o classical dogfight; equipped with only your gun, you have 
      to rely on your maneuvering and tactical skills
    o team; fight together, steal other teams's treasures 
      (involves flying around with a ball in a string, much like 
      in Thrust) and blow up their targets (which are, no doubt,  
      heavily guarded)
    o all out nuclear war; chose carefully between more than 
      twenty weapon and defense systems to stay alive and annihilate 
      your enemies
    o race; make it through the deadly course before your opponents
    * Adjustable gravity; adjustable by putting special attractors 
      or deflectors in the world, or by adjusting the global gravity in 
      various ways.
    * Cannons and personalized and vengeful robot fighters give you 
      a hard time.
    * Watch your energy, and remember to 'dock' with a fuel station 
      to refuel before it's too late.
    * Defend your home base, or terrorize and steal someone else's.
    * Equip your ship with the 15+ defense and weapon systems: 
      afterburners, cloaking devices, sensors, transporters, extra cannons, 
      mines and bombs, rockets (smarts, torpedos and nuclear), ECM, laser, 
      extra tanks, autopilot etc.

To start playing, you need to connect to a server by using a client program 
called xpilot. There are always servers running if you check with the meta 
server, but if you for some reason do not want to join them, you'll have to
start a server of your own (see man-page xpilots(6)).


%prep
%setup
%patch

%build
configure --path=/usr/local/bin
make

%install
make install

rm -rf $RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/local/bin/xpilot
/usr/local/bin/xpilots
/usr/local/bin/xp-replay
/usr/local/bin/xp-mapedit

%changelog
* Mon Sep 15 2003 root <root@Darel.com>
- Initial build.


