import typing

from sc2.unit import Unit

from sc2.position import Point2

from Python.CombatScripts.siege_tank_combat import SiegeTankCombat
from Python.CombatScripts.viking_fighter_combat import VikingFighterCombat
from sc2.ids.unit_typeid import UnitTypeId
from sc2.units import Units

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class ArmyManager:
    def __init__(self, bot: 'MyBot') -> None:
        self.bot = bot
        self.unit_exclusion_list = self.bot.information_manager.units_to_ignore_for_army
        self.viking_manager = VikingFighterCombat(bot)
        self.tank_manager = SiegeTankCombat(bot)
        self.attacking = False
        self.rally_point: Point2 = self.bot.start_location

    # TODO: Analyze the map to determine where to attack
    # TODO: Implement squads
    def __units_to_include(self) -> Units:
        return self.bot.units.exclude_type(self.unit_exclusion_list)

    def manage_army(self) -> None:
        if self.__units_to_include().amount > 20 or self.attacking:
            self.attacking = True
            self.attack_enemy_base()

        if self.__units_to_include().amount < 20 and self.attacking:
            self.attacking = False
            for unit in self.__units_to_include():
                unit.move(self.rally_point)

        self.defend_structures()

    def defend_structures(self) -> None:
        for building in self.bot.structures:
            if any (self.bot.enemy_units.closer_than(10, building)):
                position = building.position
                combat_units = self.__units_to_include()
                for unit in combat_units:
                    unit.attack(position)
                break

    def attack_enemy(self):
        for unit in self.bot.units:
            if len(self.bot.enemy_units) > 0:
                pos = self.bot.enemy_units[0].position
                self.attack_with_unit(unit, pos)

    def attack_enemy_base(self):
        position = self.bot.enemy_start_locations[0] \
            if self.bot.enemy_structures.empty \
            else self.bot.enemy_structures.closest_to(self.bot.start_location).position

        for unit in self.__units_to_include():
            self.attack_with_unit(unit, position)

    def attack_with_unit(self, unit:Unit , position):
        unit.attack(position)
        match unit.type_id:
            case UnitTypeId.VIKINGFIGHTER:
                self.viking_manager.manage_unit(unit)
            case UnitTypeId.SIEGETANK | UnitTypeId.SIEGETANKSIEGED:
                self.tank_manager.manage_unit(unit)
