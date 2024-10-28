from sc2.bot_ai import BotAI

class MyBot(BotAI):
    async def on_step(self, iteration: int):
       await self.distribute_workers()

