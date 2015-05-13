#!/usr/bin/python3
# -*- coding: utf-8 -*-
import configparser
import math
import sys

OVERHEAT_TEMPERATURE = 1200
BURN_TEMPERATURE = 1500
SOLAR_POWER = 3.094

def print_u(str):
	bytes_str = (str + "\n").encode('utf8')
	sys.stdout.buffer.write(bytes_str)



class Ship:

	name = "Unnamed"
	min_heatsink_ratio = 0.0
	max_heatsink = 0.0
	min_heatsink = 0.0
	passive_power = 0.0
	active_power = 0.0
	boosting_power = 0.0
	firing_power = 0.0
	heat_capacity = 0.0
	max_passive_equilibrium = 0.0
	max_active_equilibrium = 0.0
	max_boosting_equilibrium = 0.0
	max_firing_equilibrium = 0.0
	max_all_equilibrium = 0.0

	min_passive_equilibrium = 0.0
	min_active_equilibrium = 0.0
	min_boosting_equilibrium = 0.0
	min_firing_equilibrium = 0.0
	min_all_equilibrium = 0.0

	def __init__(self, path):
		config = configparser.ConfigParser()
		config.read(path)
		for section in config.sections():
			count = 1.
			if "count" in config[section]:
				count = float(config[section]["count"])
			for key in config[section]:
				value = config[section][key]
				if key == "shipname":
					self.name = value
				elif key == "minheatsinkratio":
					self.min_heatsink_ratio = float(value)
				elif key == "maxheatsink":
					self.max_heatsink += count * float(value)
					self.min_heatsink += count * float(value) * self.min_heatsink_ratio
				elif key == "heatcapacity":
					self.heat_capacity += count * float(value)
				elif key == "passivepower":
					self.passive_power += count * float(value)
				elif key == "activepower":
					self.active_power += count * float(value)
				elif key == "boostingpower":
					self.boosting_power += count * float(value)
				elif key == "firingpower":
					self.firing_power += count * float(value)
				elif key == "count":
					# Already treated
					pass
				else:
					print("unknown key "+key)

	def compute_equilibrium(self, power, surface):
		# Radiation in KJ = surface * 5.670373e-8 * FMath::Pow(Temperature, 4) / 1000
		# Production in KJ = power
		# Equilibrium when production equals radiation
		return math.pow(1000 * power / (surface * 5.60373e-8), 1/4)



	def compute_boost_duration(self, initial_temperature, final_temperature, power, surface, heat_capacity):
		# Radiation in KJ = surface * 5.670373e-8 * FMath::Pow(Temperature, 4) / 1000
		# Production in KJ = power
		# temperature variation is : dT/dt = (1000 * power - surface * 5.670373e-8 * FMath::Pow(T, 4)) / heat_capacity
		# T(t) = 1000 * power t - (surface * k * FMath::Pow(T, 5)/(5 * heat_capacity)

		if self.compute_equilibrium(power, surface) < final_temperature:
			# The final temperature will never be reach
			return -1;

		delta_seconds = 0.001
		time = 0.0;
		heat = initial_temperature * heat_capacity
		temperature = heat / heat_capacity

		while temperature < final_temperature:
			heat = heat + (power - surface *  5.670373e-8 * math.pow(temperature, 4) / 1000) * delta_seconds
			time = time + delta_seconds
			temperature = heat / heat_capacity

		return time

	def compute_cooling_duration(self, initial_temperature, final_temperature, power, surface, heat_capacity):
		# Radiation in KJ = surface * 5.670373e-8 * FMath::Pow(Temperature, 4) / 1000
		# Production in KJ = power
		# temperature variation is : dT/dt = (1000 * power - surface * 5.670373e-8 * FMath::Pow(T, 4)) / heat_capacity
		# T(t) = 1000 * power t - (surface * k * FMath::Pow(T, 5)/(5 * heat_capacity)

		if self.compute_equilibrium(power, surface) > final_temperature:
			# The final temperature will never be reach
			return -1;

		delta_seconds = 0.001
		time = 0.0;
		heat = initial_temperature * heat_capacity
		temperature = heat / heat_capacity

		while temperature > final_temperature:
			heat = heat + (power - surface *  5.670373e-8 * math.pow(temperature, 4) / 1000) * delta_seconds
			time = time + delta_seconds
			temperature = heat / heat_capacity

		return time


	def compute(self):
		max_solar_power = self.max_heatsink * SOLAR_POWER * 0.5
		active_max_usage = 0.26
		self.max_passive_equilibrium = self.compute_equilibrium(self.passive_power + max_solar_power, self.max_heatsink)
		self.max_active_equilibrium = self.compute_equilibrium(self.passive_power + self.active_power * active_max_usage + max_solar_power, self.max_heatsink)
		self.max_boosting_equilibrium = self.compute_equilibrium(self.passive_power + self.active_power * active_max_usage + self.boosting_power + max_solar_power, self.max_heatsink)
		self.max_firing_equilibrium = self.compute_equilibrium(self.passive_power + self.firing_power + max_solar_power, self.max_heatsink)
		self.max_all_equilibrium = self.compute_equilibrium(self.passive_power + self.active_power * active_max_usage + self.boosting_power + self.firing_power + max_solar_power, self.max_heatsink)

		min_solar_power = self.min_heatsink * SOLAR_POWER * 0.5
		self.min_passive_equilibrium = self.compute_equilibrium(self.passive_power + min_solar_power, self.min_heatsink)
		self.min_active_equilibrium = self.compute_equilibrium(self.passive_power + self.active_power * active_max_usage + min_solar_power, self.min_heatsink)
		self.min_boosting_equilibrium = self.compute_equilibrium(self.passive_power + self.active_power * active_max_usage + self.boosting_power + min_solar_power, self.min_heatsink)
		self.min_firing_equilibrium = self.compute_equilibrium(self.passive_power + self.firing_power + min_solar_power, self.min_heatsink)
		self.min_all_equilibrium = self.compute_equilibrium(self.passive_power + self.active_power * active_max_usage + self.boosting_power + self.firing_power + min_solar_power, self.min_heatsink)

		self.passive_boost_duration = self.compute_boost_duration(self.max_passive_equilibrium, OVERHEAT_TEMPERATURE, self.passive_power + self.active_power * active_max_usage + self.boosting_power + max_solar_power, self.max_heatsink, self.heat_capacity)
		self.active_boost_duration = self.compute_boost_duration(self.max_active_equilibrium, OVERHEAT_TEMPERATURE, self.passive_power + self.active_power * active_max_usage + self.boosting_power + max_solar_power, self.max_heatsink, self.heat_capacity)

		self.passive_firing_duration = self.compute_boost_duration(self.max_passive_equilibrium, OVERHEAT_TEMPERATURE, self.passive_power + self.firing_power + max_solar_power, self.max_heatsink, self.heat_capacity)
		self.active_firing_duration = self.compute_boost_duration(self.max_active_equilibrium, OVERHEAT_TEMPERATURE, self.passive_power + self.active_power * active_max_usage + self.firing_power + max_solar_power, self.max_heatsink, self.heat_capacity)

		self.burning_to_overheat_cooling = self.compute_cooling_duration(BURN_TEMPERATURE, OVERHEAT_TEMPERATURE, self.passive_power + max_solar_power, self.max_heatsink, self.heat_capacity)
		self.boosting_to_active_cooling = self.compute_cooling_duration(self.max_boosting_equilibrium, self.max_active_equilibrium, self.passive_power + max_solar_power, self.max_heatsink, self.heat_capacity)


	def dump(self):
		print("-------------------")
		print("Ship " + self.name)
		print("-------------------")
		print_u("Heat capacity: "+ str(self.heat_capacity) + " KJ/°K")
		print_u("Solar power: "+ str(SOLAR_POWER) + " KW/m²")
		print_u("Min heatsink ratio: "+ str(self.min_heatsink_ratio))

		print("Heatsink")
		print_u("  - Maximum: "+ str(self.max_heatsink) + " m²")
		print_u("  - Minimum: "+ str(self.min_heatsink) + " m²")
		print("Heat production")
		print("  - Passive: "+ str(self.passive_power) + " KW")
		print("  - Active: "+ str(self.active_power) + " KW")
		print("  - Boosting: "+ str(self.boosting_power) + " KW")
		print("  - Firing: "+ str(self.firing_power) + " KW")

		print("Equilibium at max heatsink")
		print_u("  - Passive: "+ str(self.max_passive_equilibrium) + " °K")
		print_u("  - Active: "+ str(self.max_active_equilibrium) + " °K")
		print_u("  - Boosting: "+ str(self.max_boosting_equilibrium) + " °K")
		print_u("  - Firing: "+ str(self.max_firing_equilibrium) + " °K")
		print_u("  - All: "+ str(self.max_all_equilibrium) + " °K")

		print("Equilibium at min heatsink")
		print_u("  - Passive: "+ str(self.min_passive_equilibrium) + " °K")
		print_u("  - Active: "+ str(self.min_active_equilibrium) + " °K")
		print_u("  - Boosting: "+ str(self.min_boosting_equilibrium) + " °K")
		print_u("  - Firing: "+ str(self.min_firing_equilibrium) + " °K")
		print_u("  - All: "+ str(self.min_all_equilibrium) + " °K")

		print("Usage duration")
		print("  - Boosting from passive: "+ (str(self.passive_boost_duration) + " s" if self.passive_boost_duration > 0 else "No overheat"))
		print("  - Boosting from active: "+ (str(self.active_boost_duration) + " s" if self.active_boost_duration > 0 else "No overheat"))
		print("  - Firing from passive: "+ (str(self.passive_firing_duration) + " s" if self.passive_firing_duration > 0 else "No overheat"))
		print("  - Firing from active: "+ (str(self.active_firing_duration) + " s" if self.active_firing_duration > 0 else "No overheat"))

		print("Cooling duration")
		print("  - Burning to Overheat: "+ str(self.burning_to_overheat_cooling) + " s")
		print("  - Boosting to active: "+ str(self.boosting_to_active_cooling) + " s")

ship = Ship("ghoul.ship")
ship.compute()
ship.dump()

ship = Ship("omen.ship")
ship.compute()
ship.dump()

ship = Ship("invader.ship")
ship.compute()
ship.dump()
