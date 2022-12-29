Faster-Than-Light (FTL) Tour of the Solar System by David Cornett
	OpenGL graphical simulation with close-to-accurate scale of solar system, where the player can pass by all planets at extremely high speeds.
	The program shows how quickly planets fly by at different player-selected speeds, and demonstrates how relatively 'slow' the speed of light
	is given theenourmous size of the solar system.  The planets are scaled up in size in relation to the Sun to increase visibility, but are
	proportionally accurate with respect each other.

Objects viewable on screen: 
•	Spaceship - Engine exhaust glows with increasing intensity based on player velocity (difficult to see close to the sun)
			  - To slow down, spaceship flips around forward axis so the engine exhaust faces forward
•	Sol		  - 5 Directional lights pointed along the axis of planets represent sunlight coming from the sun's center and perimeter 
			  -	Attenuation allows bright sunlight for inner solar system and dim light for outer planets
•	Planets   - Sphere objects with high res texture mapping from https://www.solarsystemscope.com/textures/
			  - Set WhichTexture global variable HIGH for 8k (where possible) or NORMAL for 2-4k for faster loading
•	Stars     - Procedurally generated, are unmoving with respect to the player
			  - White in color when player is at rest.  Stars in front of the player blueshift with increasing intensity as the player accelerates, 
				while the stars behind the player correspondingly redshift. 
			  - Amount of stars set as 100 in NUM_STARS const.  Can increase, but it becomes harder to see approaching planets


Controls: 
•	Player can set the speed of the spaceship as a multiple or percentage of c, the speed of light
•   'w' key to increase speed, 's' key to decrease
•   Max speed allowed is 134c, enabling transit from Sol to Neptune in just under 2 minutes
•   Current speed (as c multiple) displayed on the screen
•	Player can right click to bring up menu options:
		- 'Go Lightspeed': immediately accelerate (or decelerate) to lightspeed.  At this speed, it will take a long time to go between the planets, but
		                   it allows the player to see planets well when passing by.  Recommendation: only use this option when already by a planet.
		- 'Reset': Restart program from the spaceship's initial position by the Sun
		- 'Quit': exit program immediately


Physics Mechanics: 
•   Spaceship and background stars are stationary, with the Sun and 8 planets moving toward the player at various speeds
•   The program mostly ignores relativistic physics. Time dilation and mass of observer would approach infinity close to c, and in fact the player can go much faster than c to make the simulation enjoyable.   
		- Otherwise, it would take 8 min. to get to Earth from Sol, or 4 hours to Neptune
•	Newton's Third Law (every action has an equal and opposite reaction): 
		- After a period of acceleration, if the player decelerates, the spaceship first rotates 180 degrees so the engine points the correct way.
		- After deceleration, if the player begins to accelerate, the spaceship rotates again.
