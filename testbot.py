from sc2.bot_ai import BotAI

class MyBot(BotAI):
    async def on_step(self, iteration: int):
        print(f"iteration: {iteration}")
        rndwrk = self.workers.random
        closestMineral = self.state.mineral_field.closest_to(rndwrk)
        await self.do(rndwrk.gather(closestMineral))


