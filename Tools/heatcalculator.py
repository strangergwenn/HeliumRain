#!/usr/bin/python3
import configparser
import math

OVERHEAT_TEMPERATURE = 1200
BURN_TEMPERATURE = 1500
SOLAR_POWER = 3.094

class Ship:

	name = "Unnamed"
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
				elif key == "maxheatsink":
					self.max_heatsink += count * float(value)
				elif key == "minheatsink":
					self.min_heatsink += count * float(value)
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


	def dump(self):
		print("-------------------")
		print("Ship " + self.name)
		print("-------------------")
		print("Heat capacity: "+ str(self.heat_capacity) + " KJ/°K")
		print("Solar power: "+ str(SOLAR_POWER) + " KW/m²")

		print("Heatsink")
		print("  - Maximum: "+ str(self.max_heatsink) + " m²")
		print("  - Minimum: "+ str(self.min_heatsink) + " m²")
		print("Heat production")
		print("  - Passive: "+ str(self.passive_power) + " KW")
		print("  - Active: "+ str(self.active_power) + " KW")
		print("  - Boosting: "+ str(self.boosting_power) + " KW")
		print("  - Firing: "+ str(self.firing_power) + " KW")

		print("Equilibium at max heatsink")
		print("  - Passive: "+ str(self.max_passive_equilibrium) + " °K")
		print("  - Active: "+ str(self.max_active_equilibrium) + " °K")
		print("  - Boosting: "+ str(self.max_boosting_equilibrium) + " °K")
		print("  - Firing: "+ str(self.max_firing_equilibrium) + " °K")
		print("  - All: "+ str(self.max_all_equilibrium) + " °K")

		print("Equilibium at min heatsink")
		print("  - Passive: "+ str(self.min_passive_equilibrium) + " °K")
		print("  - Active: "+ str(self.min_active_equilibrium) + " °K")
		print("  - Boosting: "+ str(self.min_boosting_equilibrium) + " °K")
		print("  - Firing: "+ str(self.min_firing_equilibrium) + " °K")
		print("  - All: "+ str(self.min_all_equilibrium) + " °K")




ship = Ship("ghoul.ship")
ship.compute()
ship.dump()
