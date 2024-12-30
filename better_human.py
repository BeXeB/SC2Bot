import math

from sc2.bot_ai import BotAI
from sc2.ids.unit_typeid import UnitTypeId

from result_saver import save_human_result
from state_translator import translate_state
from testbot import STEPS_PER_SECOND


class BetterHuman(BotAI):
    async def on_start(self):
        self.CC_BUILD_TIME_SECONDS: int = math.ceil(
            self.game_data.units[UnitTypeId.COMMANDCENTER.value]._proto.build_time / STEPS_PER_SECOND)
        self.CC_TRAVEL_TIME_SECONDS: int = 5
        self.REFINERY_BUILD_TIME_SECONDS: int = math.ceil(
            self.game_data.units[UnitTypeId.REFINERY.value]._proto.build_time / STEPS_PER_SECOND)
        self.REFINERY_TRAVEL_TIME_SECONDS: int = 1
        self.WORKER_BUILD_TIME_SECONDS: int = math.ceil(
            self.game_data.units[UnitTypeId.SCV.value]._proto.build_time / STEPS_PER_SECOND)
        self.SUPPLY_BUILD_TIME_SECONDS: int = math.ceil(
            self.game_data.units[UnitTypeId.SUPPLYDEPOT.value]._proto.build_time / STEPS_PER_SECOND)
        self.SUPPLY_TRAVEL_TIME_SECONDS: int = 2
        self.busy_workers = {}
        self.time_limit = 0

    async def on_step(self, iteration: int) -> None:
        pass

    async def on_end(self, iteration: int) -> None:
        end_state = translate_state(self)
        save_human_result(self, end_state, self.time)