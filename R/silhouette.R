silhouette <- function(x, ...) UseMethod("silhouette")

## Accessor and more:
silhouette.partition <- function(x, ...) {
    r <- x$silinfo$widths
    if(is.null(r))
        stop("invalid partition object")
    attr(r, "Ordered") <- TRUE # (cluster <increasing>, s.i <decreasing>)
    attr(r, "call") <- x$call
    class(r) <- "silhouette"
    r
}

silhouette.default <- function(x, dist, ...) {
    if(missing(dist)) stop("Need a dissimilarity (as 2nd argument)")
    cll <- match.call()
    dist <- as.dist(dist) # hopefully
    if(!is.null(cl <- x$clustering)) x <- cl
    n <- length(x)
    if(n != attr(dist, "Size"))
        stop("clustering `x' and dissimilarity `dist' are incompatible")
    if(!all(x == round(x))) stop("`x' must only have integer codes")
    k <- length(clid <- sort(unique(x)))
    if(k <= 1 || k >= n)
        return(NA)
    dmat <- as.matrix(dist)# so we can apply(.) below
    wds <- matrix(NA, n,3, dimnames =
                  list(names(x), c("cluster","neighbor","sil_width")))
    for(j in 1:k) {
        Nj <- sum(iC <- x == clid[j])
        wds[iC, 1] <- j
        a.i <- if(Nj > 1) colSums(dmat[iC, iC])/(Nj - 1) else 0
        diC <- apply(dmat[!iC, iC], 2, function(r) tapply(r, x[!iC], mean))
        wds[iC,"neighbor"]  <- clid[-j][minC <- max.col(-t(diC))]
        b.i <- diC[cbind(minC, seq(minC))]
        s.i <- (b.i - a.i) / pmax(b.i, a.i)
        wds[iC,"sil_width"] <- s.i
    }
    attr(wds, "Ordered") <- FALSE
    attr(wds, "call") <- cll
    class(wds) <- "silhouette"
    wds
}


### Maybe we should use  `sort()' instead  {{which should become *generic*?}
## Order <- function(x, ...) UseMethod("Order")
sortSilhouette <- function(object, ...) {
    if(attr(object,"Ordered")) return(object)
    ## Else :
    if(is.null(n <- nrow(object)) || n < 1)
        stop("invalid silhouette structure")
    if(is.null(rownames(object)))
        rownames(object) <- as.character(1:n)
    k <- length(clid <- sort(unique(cl <- object[,"cluster"]))) # cluster ID s
    r <- object[order(cl, - object[,"sil_width"]) , , drop = FALSE]
    attributes(r) <- attributes(object) # but:
    attr(r,"Ordered") <- TRUE
    r
}

summary.silhouette <- function(object, FUN = mean, ...)
{
    if(ncol(object) != 3) stop("invalid `silhouette' object")
    n <- nrow(object)
    cl <- object[, "cluster"]
    si <- object[, "sil_width"]
    r <- list(si.summary = summary(si, ...),
              clus.avg.widths = tapply(si, cl, FUN),
              clus.sizes = table(cl),
              avg.width = FUN(si),
              call = attr(object,"call"),
              Ordered = attr(object,"Ordered"))
    class(r) <- "summary.silhouette"
    r
}

print.summary.silhouette <- function(x, ...)
{
    k <- length(csiz <- x$clus.sizes)
    cat("Silhouette of", sum(csiz), "units in", k, "clusters",
        if(!is.null(x$call)) paste("from", deparse(x$call)),
        ":\nCluster sizes and average silhouette widths:\n")
    cwid <- x$clus.avg.widths
    names(cwid) <- csiz
    print(cwid, ...)
    cat("Individual silhouette widths:\n")
    print(x$si.summary, ...)
    invisible(x)
}


## This was the internal function silhouPlot() in plot.partition() :
plot.silhouette <-
    function(x, nmax.lab = 40, max.strlen = 5,
             main = NULL, sub = NULL,
             xlab = expression("Silhouette width " * s[i]),
             col = heat.colors(n), cex.names = par("cex.axis"),
             do.n.k = TRUE, do.clus.stat = TRUE, ...)
{
    if(!is.matrix(x) || ncol(x) != 3)
        stop("No valid silhouette information (#{clusters} =? 1)") #
    x <- sortSilhouette(x)
    n <- length(s <- rev(x[, "sil_width"]))
    space <- c(0, rev(diff(cli <- x[, "cluster"])))
    space[space != 0] <- 0.5 # gap between clusters
    names <- if(n < nmax.lab)
        substring(rev(rownames(x)), 1, max.strlen)
    if(is.null(main)) {
        main <- "Silhouette plot"
        if(!is.null(cll <- attr(x,"call"))) { # drop initial "silhouette":
            if(!is.na(charmatch("silhouette", deparse(cll[[1]]))))
                cll[[1]] <- as.name("FF")
            main <-  paste(main, "of", sub("^FF","", deparse(cll)))
        }
    }
    sum <- summary(x)
    k <- length(nj <- sum$clus.sizes)
    if(is.null(sub))
        sub <- paste("Average silhouette width : ",
                     round(sum$avg.width, digits = 2))
    y <- barplot(s, space = space, names = names, xlab = xlab,
                 xlim = c(min(0, min(s)), 1),
                 horiz = TRUE, las = 1, mgp = c(2.5, 1, 0),
                 col = col, cex.names = cex.names, ...)
    title(main = main, sub = sub, adj = 0)
    if(do.n.k) {
        mtext(paste("n =", n),  adj = 0)
        mtext(substitute(k ~~ "clusters" ~~ C[j], list(k=k)), adj= 1)
    }
    if(do.clus.stat) {
        mtext(expression(paste(j," :  ", n[j]," | ", ave[i %in% Cj] ~~ s[i])),
              adj = 1.05, line = -1.2)
        y <- rev(y)
        for(j in 1:k) {
            yj <- mean(y[cli == j])
            text(1, yj, paste(j,":  ", nj[j]," | ",
                              format(sum$clus.avg.widths[j], digits = 2)),
                 xpd = NA, adj = 0.8)
        }
    }
}