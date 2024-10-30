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
        cur, rem = BB.CalculateAllCurrentAndRemainingMinerals(self, [])

        BB.TestCalculations(cur, rem)
        #cur, rem = BB.CalculateAllCurrentAndRemainingMinerals(self)
        #BB.TestCalculations(cur, rem)
        #BB.FindOptimalNewBase(self)





