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
                subprocess.call([self.executable], 
                                stdout=subprocess.DEVNULL, 
                                stderr=subprocess.DEVNULL)
                self.times.append(round((time.time() - start) * 10 ** 6))

            self.export.append(self.get_stats(self.times))
            self.times.clear()


    def get_stats(self, times):
        return [statistics.mean(times), round(statistics.stdev(times), 2)]


    def write_csv(self):
        with open('{}.csv'.format(self.name), 'w', newline='') as file:
            fieldnames = ['mean', 'sd']
            writer = csv.DictWriter(file, fieldnames=fieldnames)
            writer.writeheader()
            for row in self.export:
                writer.writerow({fieldnames[0]: row[0], fieldnames[1]: row[1]})
