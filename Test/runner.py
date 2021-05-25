import time
import subprocess
import statistics
import csv
import sys

class Runner:
    #nthreads = [1, 8, 16, 32, 64, 128]
    nthreads = [1,2,3,4]

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

    

    def progressbar(it, prefix="", size=60, file=sys.stdout):
        count = len(it)
        def show(j):
            x = int(size*j/count)
            file.write("%s[%s%s] %i/%i\r" % (prefix, "#"*x, "."*(size-x), j, count))
            file.flush()        
        show(0)
        for i, item in enumerate(it):
            yield item
            show(i+1)
        file.write("\n")
        file.flush()
