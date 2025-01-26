# plot x/y chart for latency in cache access
# read in time series from file with following format
# Timestamp,Latency
# - Timestamp is in millisecond since epoch
# - Latency is in ms
#
# It will also plot average, and median with corresponding values rendered along.

# accept arguments of input time seires txt file
args <- commandArgs(trailingOnly = TRUE)

if (length(args) < 2) {
	stop("Not enough input arguments provided.\nRscript plotchart.R <input-file> <output-image-file>", call. = FALSE)
}

data <- read.table(args[1], header=TRUE, sep=",")
data$Date <- as.POSIXct(data$Timestamp / 1000, origin="1970-01-01", tz="Asia/Bangkok")

avg_value <- mean(data$Latency)
median_value <- median(data$Latency)

avg_value_display <- round(avg_value, 2)
median_value_display <- round(median_value, 2)

png(args[2], width=1280, height=800)

plot(data$Date, data$Latency,
	 main="Time series of latency in grabbing a next item from ring buffer",
	 xlab="Timestamp",
	 ylab="Latency (micro)",
	 type="l", col="blue", lwd=3)

abline(h=avg_value, col="red", lty=2, lwd=3)
abline(h=median_value, col="purple", lty=3, lwd=3)

text(x=min(data$Date), y=avg_value,
	 labels=paste("Average:", avg_value_display),
	 pos=3, col="red")

text(x=max(data$Date), y=median_value,
	 labels=paste("Median:", median_value_display),
	 pos=3, col="purple")

legend("topright", legend=c("Actual", "Average", "Median"),
	   col=c("blue", "red", "purple"), lty=c(1,2,3))

dev.off()
