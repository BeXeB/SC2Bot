import math
import multiprocessing as mp

from sc2_mcts import *
class MctsRunner:
    def __init__(self):
        self.running : bool = False

        self.next_action = Action.none
        self.mcts = Mcts(state=State(),
                         seed=0,
                         rollout_depth=100,
                         exploration=math.sqrt(2),
                         value_heuristic=ValueHeuristic.UCT,
                         rollout_heuristic=RolloutHeuristic.weighted_choice)
        self.mcts_lock = mp.Lock()
        self.mcts_thread: mp.Process = mp.Process(target=self.mcts_search_thread)


    def mcts_search_thread(self):
        while self.running:
            with self.mcts_lock:
                self.mcts.search(500)
                self.next_action = self.mcts.get_best_action()

    def start(self):
        self.running = True
        self.mcts_thread.start()

    def update_state(self, state: State):
        with self.mcts_lock:
            self.mcts.update_root_state(state)

    def get_next_action(self):
            return self.next_action

    def do_action(self, new_state: State):
        with self.mcts_lock:
            self.next_action = Action.none
            self.mcts.update_root_state(new_state)
