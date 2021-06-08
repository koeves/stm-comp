import time
import subprocess
import statistics
import csv
from datetime import datetime

class Runner:
    nthreads = [1, 2, 4, 8, 16]
    #nthreads = [2,3,4]

    def __init__(self, name, inserts):
        self.name = '{}_{}'.format(name, inserts)
        self.inserts = inserts
        self.export = []
        self.executable = './{}'.format(name)
        print("Performing tests for: " + self.name)


    def run(self, repeat):
        for nthread in self.nthreads:
            print("\trunning with {} threads".format(nthread))
            times = []
            for _ in range(repeat):
                start = time.time()
                subprocess.call([self.executable, str(nthread), str(self.inserts)])
                times.append(round((time.time() - start) * 10 ** 6))

            self.export.append(self.get_stats(nthread, times))


    def get_stats(self, nthread, times):
        return [nthread, statistics.mean(times), round(statistics.stdev(times), 2)]


    def write_csv(self):
        now = datetime.now().strftime('%m-%d_%H:%M:%S')
        with open('{}_{}.csv'.format(self.name, now), 'w', newline='') as file:
            fieldnames = ['nthreads','mean', 'sd', 'speedup']
            writer = csv.DictWriter(file, fieldnames=fieldnames)
            writer.writeheader()
            for row in self.export:
                writer.writerow({fieldnames[0]: row[0], 
                                 fieldnames[1]: row[1], 
                                 fieldnames[2]: row[2],
                                 fieldnames[3]: self.export[0][1]/row[1]})

            print("Wrote csv: " + self.name + '_' + now)
            

