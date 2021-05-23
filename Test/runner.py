import time
import subprocess
import statistics

class Runner:
    def __init__(self, executable):
        self.times = []
        self.executable = executable

    def run(self, times):
        for _ in range(0, times):
            start = time.time()
            subprocess.call([self.executable], stdout=subprocess.DEVNULL)
            self.times.append(round((time.time() - start) * 10 ** 6))

    def get_stats(self):
        return statistics.mean(self.times), round(statistics.stdev(self.times), 2)