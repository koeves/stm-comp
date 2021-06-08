setwd("~/Desktop/Thesis/figures/graphs")

# EXPORT PDF: 4.77 x 7.61 in

threads = c(1,2,4,8,16)

##########################################
#                                        #
#   10K INSERTS                          #
#                                        #
##########################################

rbt_etx_10k = read.csv("rbt_etx_10000_05-30_150202.csv")
rbt_etx_10k$mean = round(rbt_etx_10k$mean / 1000)
rbt_etx_10k$sd = round(rbt_etx_10k$sd / 1000)

skip_etx_10k = read.csv("skip_etx_10k.csv")
skip_etx_10k$mean = round(skip_etx_10k$mean / 1000)
skip_etx_10k$sd = round(skip_etx_10k$sd / 1000)

skip_ctx_10k <- read.csv("skip_ctx_10k.csv")
skip_ctx_10k$mean = round(skip_ctx_10k$mean / 1000)
skip_ctx_10k$sd = round(skip_ctx_10k$sd / 1000)

# aborts
rbt_etx_aborts_10k = c()
rbt_etx_aborts_100k = c()
skip_etx_aborts_10k = c()
skip_etx_aborts_100k = c()
skip_ctx_aborts_10k = c()
for (i in c("1","2","4","8","16")) {
  file = sprintf("rbt_etx_aborts_10000_%s.txt", i)
  rbt_etx_aborts_10k = c(rbt_etx_aborts_10k, mean(read.delim(file)$X0))
  
  file = sprintf("rbt_etx_aborts_100000_%s.txt", i)
  rbt_etx_aborts_100k = c(rbt_etx_aborts_100k, mean(read.delim(file)$X0))
  
  file = sprintf("skip_etx_aborts_10k_%s.txt", i)
  skip_etx_aborts_10k = c(skip_etx_aborts_10k, mean(read.delim(file)$X0))
  
  #file = sprintf("skip_etx_aborts_100000_%s.txt", i)
  #skip_etx_aborts_100k = c(skip_etx_aborts_100k, mean(read.delim(file)$X0))
  
  file = sprintf("skip_ctx_aborts_10k_%s.txt", i)
  skip_ctx_aborts_10k = c(skip_ctx_aborts_10k, mean(read.delim(file)$X0))
}

do_10k <- function() {
  rounds = list(rbt_etx_10k, c(skip_etx_10k, skip_ctx_10k))
  names = list(c("Red-Black Tree 10k inserts", "Normalised speedup of\nRed-Black Tree 10k inserts"),
               c("Skiplist 10k inserts", "Normalised speedup of\nSkiplist 10k inserts"))
  
  par(mfrow=c(2,2))
  # EXPORT: 5.5 x 6 in
  
  # EXEC RBT
  plot.default(threads, rbt_etx_10k$mean, type="n",
       ylim=range(c(rbt_etx_10k$mean-rbt_etx_10k$sd, rbt_etx_10k$mean+rbt_etx_10k$sd)),
       pch=19,
       ylab="Mean wall clock time (ms) +/- SD",
       xlab="Number of threads",
       main="Red-Black Tree 10k inserts",
       axes=F)
  axis(side=1, at=threads, labels=threads)
  axis(side=2, at=c(min(rbt_etx_10k$mean-rbt_etx_10k$sd), 
                    rbt_etx_10k$mean, 
                    max(rbt_etx_10k$mean+rbt_etx_10k$sd)))
  lines(threads, rbt_etx_10k$mean, type="b", lty=2, pch=8)
  arrows(threads, 
         rbt_etx_10k$mean-rbt_etx_10k$sd, 
         threads, rbt_etx_10k$mean+rbt_etx_10k$sd, 
         length=0.05, angle=90, code=3)
  legend("topright", 
         legend=c("encounter-\norder"),
         lty=c(2), pch=c(8),  box.lwd = 0, bty="n", bg="transparent")
  
  # EXEC SKIP
  plot(range(threads), range(c(skip_etx_10k$mean, skip_ctx_10k$mean)), type="n",
       ylim=range(c(skip_etx_10k$mean-skip_etx_10k$sd, skip_ctx_10k$mean+skip_ctx_10k$sd)),
       pch=19,
       ylab="Mean wall clock time (ms) +/- SD",
       xlab="Number of threads",
       main="Skiplist 10k inserts",
       axes=F)
  axis(side=1, at=threads, labels = threads)
  axis(side=2, at=c(min(c(skip_etx_10k$mean-skip_etx_10k$sd, skip_ctx_10k$mean+skip_ctx_10k$sd)), 
                    c(skip_etx_10k$mean, skip_ctx_10k$mean), 
                    max(c(skip_etx_10k$mean-skip_etx_10k$sd, skip_ctx_10k$mean+skip_ctx_10k$sd))))
  lines(threads, skip_etx_10k$mean, type="b", lty=2, pch=8)
  lines(threads, skip_ctx_10k$mean, type="b", lty=5, pch=2)
  arrows(threads, 
         skip_etx_10k$mean-skip_etx_10k$sd, 
         threads, skip_etx_10k$mean+skip_etx_10k$sd, 
         length=0.05, angle=90, code=3)
  arrows(threads, 
         skip_ctx_10k$mean-skip_ctx_10k$sd, 
         threads, skip_ctx_10k$mean+skip_ctx_10k$sd, 
         length=0.05, angle=90, code=3)
  legend("topright", 
         legend=c("encounter-\norder","commit-\ntime"),
         lty=c(2,5), pch=c(8,2),  box.lwd = 0, bty="n", bg="transparent", y.intersp=0.4) 
  
  aborts = list(round(rbt_etx_aborts_10k,2), round(skip_etx_aborts_10k,2))
  abort_names = c("Abort rate of Red-Black Tree\n10k inserts", "Abort rate of Skiplist\n10k inserts")
  
  rbt_etx_aborts_10k <- round(rbt_etx_aborts_10k,2)
  skip_etx_aborts_10k <- round(skip_etx_aborts_10k,2)
  skip_ctx_aborts_10k <- round(skip_ctx_aborts_10k,2)
  
  # RBT ABORTS
  plot(range(threads), range(rbt_etx_aborts_10k),
       type="n",
       xlab="Number of threads",
       ylab="Abort rate",
       main="Abort rate of Red-Black Tree\n10k inserts",
       axes=F)
  axis(side=1, at=threads)
  axis(side=2, 
       at=round(rbt_etx_aborts_10k,2), 
       labels=paste0(round(rbt_etx_aborts_10k,2) * 100, "%"),
       las=1)
  lines(threads, rbt_etx_aborts_10k, type = "b", lty=2, pch=8)
  legend("bottomright", 
         legend=c("encounter-\norder"),
         lty=c(2), pch=c(8),  box.lwd = 0, bty="n", bg="transparent")
  
  # SKIP ABORTS
  plot(range(threads), range(c(skip_etx_aborts_10k, skip_ctx_aborts_10k)),
       type="n",
       xlab="Number of threads",
       ylab="Abort rate",
       main="Abort rate of Skiplist\n10k inserts",
       axes=F)
  axis(side=1, at=threads)
  axis(side=2, 
       at=c(skip_etx_aborts_10k, skip_ctx_aborts_10k), 
       labels=paste0(c(skip_etx_aborts_10k, skip_ctx_aborts_10k) * 100, "%"),
       las=1)
  lines(threads, skip_etx_aborts_10k, type = "b", lty=2, pch=8)
  lines(threads, skip_ctx_aborts_10k, type = "b", lty=5, pch=2)
  legend("topleft", 
         legend=c("encounter-\norder","commit-\ntime"),
         lty=c(2,5), pch=c(8,2),  box.lwd = 0, bty="n", bg="transparent", inset=c(0,-0.2), y.intersp=0.4)
}
do_10k()


##########################################
#                                        #
#   SPEEDUP                              #
#                                        #
##########################################

# EXPORT : 6 x 3.5 in

par(mfrow=c(1,2))  
# SPEEDUP RBT
plot.default(as.factor(threads), rbt_etx_10k$speedup, type="n",
     xlab="Number of threads",
     ylab="Speedup compared to 1 thread",
     main="Normalised speedup of\nRed-Black Tree 10k inserts",
     axes=F)
axis(side=1, at=as.factor(threads), labels=threads)
axis(side=2, at=round(rbt_etx_10k$speedup,2),labels=round(rbt_etx_10k$speedup,2))
lines(as.factor(threads), rbt_etx_10k$speedup, type = "b", lty=2, pch=8)
legend("bottomright", 
       legend=c("encounter-\norder"),
       lty=c(2), pch=c(8),  box.lwd = 0, bty="n", bg="transparent", inset=c(0,0.1))

# SPEEDUP SKIP
plot.default(as.factor(threads), skip_etx_10k$speedup, type="n",
     xlab="Number of threads",
     ylab="Speedup compared to 1 thread",
     main="Normalised speedup of\nSkiplist 10k inserts",
     axes=F)
axis(side=1, at=as.factor(threads), labels=threads)
axis(side=2, at=c(round(skip_etx_10k$speedup,2),round(skip_ctx_10k$speedup,2)))
lines(as.factor(threads), skip_etx_10k$speedup, type = "b", lty=2, pch=8)
lines(as.factor(threads), skip_ctx_10k$speedup, type = "b", lty=5, pch=2)
legend("topleft", 
       legend=c("encounter-\norder","commit-\ntime"),
       lty=c(2,5), pch=c(8,2),  box.lwd = 0, bty="n", bg="transparent", y.intersp=1.8)


##########################################
#                                        #
#   PROCESS EXEC                         #
#                                        #
##########################################

# export : 8.5 x 5 in

par(mfrow=c(1,2))

etx_ops <- read.delim("etx-ops.txt")
names(etx_ops) <- c("operation", "time")
etx_means <- c(mean(etx_ops$time[etx_ops$operation == "BEGIN:"]),
         mean(etx_ops$time[etx_ops$operation == "READ:"]),
         mean(etx_ops$time[etx_ops$operation == "WRITE:"]),
         mean(etx_ops$time[etx_ops$operation == "COMMIT:"]),
         mean(etx_ops$time[etx_ops$operation == "ABORT:"]))

etx_rel_times <- c(etx_means[1]/sum(etx_means),
                   etx_means[2]/sum(etx_means),
                   etx_means[3]/sum(etx_means),
                   etx_means[4]/sum(etx_means),
                   etx_means[5]/sum(etx_means))

etx <- matrix(etx_rel_times*100, nrow=1, ncol=5)
rownames(etx) <- c("commit-time")
colnames(etx) <- c("begin","read","write","commit","abort")

ctx_ops <- read.delim("ctx-ops.txt")
names(ctx_ops) <- c("operation", "time")
ctx_means <- c(mean(ctx_ops$time[ctx_ops$operation == "BEGIN:"]),
               mean(ctx_ops$time[ctx_ops$operation == "READ:"]),
               mean(ctx_ops$time[ctx_ops$operation == "WRITE:"]),
               mean(ctx_ops$time[ctx_ops$operation == "COMMIT:"]),
               mean(ctx_ops$time[ctx_ops$operation == "ABORT:"]))

ctx_rel_times <- c(ctx_means[1]/sum(ctx_means),
                   ctx_means[2]/sum(ctx_means),
                   ctx_means[3]/sum(ctx_means),
                   ctx_means[4]/sum(ctx_means),
                   ctx_means[5]/sum(ctx_means))

ctx <- matrix(ctx_rel_times*100, nrow=1, ncol=5)
rownames(ctx) <- c("commit-time")
colnames(ctx) <- c("begin","read","write","commit","abort")

par(mgp=c(3,0.75,0.5))

etx_plot <- barplot(etx, beside=T, density=c(5,10,20,30,7) , angle=c(36,45,90,11,0),
        xlab="Operation", ylab="Relative execution time [%]",
        main="Encounter-order transaction\noperation times", xaxt = 'n', ylim = c(0,75))
legend("topleft",legend=c("begin","read","write","commit","abort"),
       box.lwd = 0, bty="n", bg="transparent", density=c(5,10,20,30,7), angle=c(36,45,90,11,0),
       inset=c(-0.05,0), cex = 0.8, y.intersp=1.2)
axis(side=1, at=etx_plot, labels=colnames(etx), las=2, tick = F, cex.axis=0.8, mgp=c(3,0,0.5))
text(x=etx_plot, y=etx, label=paste0(round(etx_rel_times*100),"%"), pos=3)

ctx_plot <- barplot(ctx, beside=T, density=c(5,10,20,30,7) , angle=c(36,45,90,11,0),
        xlab="Operation", ylab="Relative execution time [%]",
        main="Commit-time transaction\noperation times", xaxt = 'n', ylim=c(0,75))
legend("topleft",legend=c("begin","read","write","commit","abort"),
       box.lwd = 0, bty="n", bg="transparent", density=c(5,10,20,30,7), angle=c(36,45,90,11,0),
       inset=c(-0.05,0), cex = 0.8, y.intersp=1.2)
axis(side=1, at=ctx_plot, labels=colnames(ctx), las=2, tick = F, cex.axis=0.8, mgp=c(3,0,0.5))
text(x=ctx_plot, y=ctx, label=paste0(round(ctx_rel_times*100),"%"), pos=3)

par(mgp=c(3,1,0))











