import time
import subprocess
import statistics
import csv

class Runner:
    def __init__(self, name):
        self.times = []
        self.export = []
        self.name = name
        self.executable = './{}'.format(name)


    def run(self, times):
        for _ in range(0, 100):
            for _ in range(0, times):
                start = time.time()
                subprocess.call([self.executable], stdout=subprocess.DEVNULL)
                self.times.append(round((time.time() - start) * 10 ** 6))

            self.export.append(self.get_stats())
            self.times.clear()


    def get_stats(self):
        return [statistics.mean(self.times), round(statistics.stdev(self.times), 2)]


    def write_csv(self):
        with open('{}.csv'.format(self.name), 'w', newline='') as file:
            fieldnames = ['mean', 'sd']
            writer = csv.DictWriter(file, fieldnames=fieldnames)
            writer.writeheader()
            for row in self.export:
                writer.writerow({'mean': row[0], 'sd': row[1]})
