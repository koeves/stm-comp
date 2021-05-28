import time
import subprocess
import statistics
import csv

class Runner:
    nthreads = [1, 4, 8, 16, 32]
    #nthreads = [1,2,3,4]

    def __init__(self, name):
        print("Performing tests for: " + name)
        self.name = name
        self.export = []
        self.executable = './{}'.format(name)


    def run(self, repeat):
        for nthread in self.nthreads:
            print("\trunning with {} threads".format(nthread))
            times = []
            for _ in range(repeat):
                start = time.time()
                subprocess.call([self.executable, str(nthread)], 
                                stdout=subprocess.DEVNULL, 
                                stderr=subprocess.DEVNULL)
                times.append(round((time.time() - start) * 10 ** 6))

            self.export.append(self.get_stats(nthread, times))


    def get_stats(self, nthread, times):
        return [nthread, statistics.mean(times), round(statistics.stdev(times), 2)]


    def write_csv(self):
        with open('{}.csv'.format(self.name), 'w', newline='') as file:
            fieldnames = ['nthreads','mean', 'sd']
            writer = csv.DictWriter(file, fieldnames=fieldnames)
            writer.writeheader()
            for row in self.export:
                writer.writerow({fieldnames[0]: row[0], fieldnames[1]: row[1], fieldnames[2]: row[2]})
            print("Wrote csv: " + self.name)
            

