library(cluster)

## generate 1500 objects, divided into 2 clusters.
if(R.version$major != "1" || as.numeric(R.version$minor) >= 7) RNGversion("1.6")
set.seed(264)
x <- rbind(cbind(rnorm(700, 0,8), rnorm(700, 0,8)),
           cbind(rnorm(800,50,8), rnorm(800,10,8)))

isEq <- function(x,y, epsF = 100)
    is.logical(r <- all.equal(x,y, tol = epsF * .Machine$double.eps)) && r

.proctime00 <- proc.time()

## full size sample {should be = pam()}:
n0 <- length(iSml <- c(1:70, 701:720))
summary(clara0 <- clara(x[iSml,], k = 2, sampsize = n0))
          pam0 <- pam  (x[iSml,], k = 2)
stopifnot(identical(clara0$clustering, pam0$clustering)
        , isEq(clara0$objective, unname(pam0$objective[2]))
          )

summary(clara2 <- clara(x, 2))

clInd <- c("objective", "medoids", "clusinfo")
clIn4 <- c(clInd, "sample")
## clara() {as original code} always draws the *same* random samples !!!!
clara(x, 2, samples = 50)[clInd]
for(i in 1:20)
    print(clara(x[sample(nrow(x)),], 2, samples = 50)[clInd])

clara(x, 2, samples = 101)[clInd]
clara(x, 2, samples = 149)[clInd]
clara(x, 2, samples = 200)[clInd]
## Note that this last one is practically identical to the slower  pam() one

x[print(sample(length(x), 20))] <- NA
clara(x, 2, samples = 50)[clInd]

###-- Larger example: 2000 objects, divided into 5 clusters.
x5 <- rbind(cbind(rnorm(400, 0,4), rnorm(400, 0,4)),
            cbind(rnorm(400,10,8), rnorm(400,40,6)),
            cbind(rnorm(400,30,4), rnorm(400, 0,4)),
            cbind(rnorm(400,40,4), rnorm(400,20,2)),
            cbind(rnorm(400,50,4), rnorm(400,50,4)))
## plus 1 random dimension
x5 <- cbind(x5, rnorm(nrow(x5)))

clara(x5, 5)
summary(clara(x5, 5, samples = 50))
## 3 "half" samples:
clara(x5, 5, samples = 999)
clara(x5, 5, samples = 1000)
clara(x5, 5, samples = 1001)

clara(x5, 5, samples = 2000)#full sample

###--- Start a version of  example(clara) -------

## xclara : artificial data with 3 clusters of 1000 bivariate objects each.
data(xclara)
(clx3 <- clara(xclara, 3))
## Plot similar to Figure 5 in Struyf et al (1996)
plot(clx3)

## The  rngR = TRUE case is currently in the non-strict tests
## ./clara-ex.R
## ~~~~~~~~~~~~

###--- End version of example(clara) -------

## Last Line:
cat('Time elapsed: ', proc.time() - .proctime00,'\n')