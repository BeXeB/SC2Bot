from sc2.bot_ai import BotAI
from Actions.BuildBase import BaseBuilder

class MyBot(BotAI):
    async def on_step(self, iteration: int):
        print(f"iteration: {iteration}")

        #rndwrk = self.workers.random
        #closestMineral = self.state.mineral_field.closest_to(rndwrk)
        #if not rndwrk.is_gathering:
        #    await self.do(rndwrk.gather(closestMineral))
        BB = BaseBuilder()
        cur, rem = BB.CalculateAllCurrentAndRemainingMinerals(self)
        BB.TestCalculations(cur, rem)

        #print(self.state.mineral_field.closest_distance_to(self.start_location))
        #print(self.state.mineral_field.furthest_distance_to(self.start_location))




