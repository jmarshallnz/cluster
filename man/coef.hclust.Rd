\name{coef.hclust}
\alias{coef.hclust}
\alias{coef.twins}
\title{Agglomerative Coefficient for 'hclust' Objects}
\description{
  Computes the \dQuote{agglomerative coefficient}, measuring the
  clustering structure of the dataset.

  For each observation i, denote by \eqn{m(i)} its dissimilarity to the
  first cluster it is merged with, divided by the dissimilarity of the
  merger in the final step of the algorithm.  The agglomerative
  coefficient is the average of all \eqn{1 - m(i)}.  It can also be seen
  as the average width (or the percentage filled) of the banner plot.

  Because it grows with the number of observations, this measure should not
  be used to compare datasets of very different sizes.
}
\usage{
coef.hclust(object, \dots)
\method{coef}{hclust}(object, \dots)
\method{coef}{twins}(object, \dots)
}
\arguments{
  \item{object}{an object of class \code{"hclust"} or \code{"twins"},
    i.e., typically the result of
    \code{\link{hclust}(.)},\code{\link{agnes}(.)}, or \code{\link{diana}(.)}.

    Since \code{coef.hclust} only uses \code{object$heights}, and
    \code{object$merge}, \code{object} can be any list-like object with
    appropriate \code{merge} and \code{heights} components.
  }
  \item{\dots}{currently unused potential further arguments}
}
\value{
  a number specifying the \emph{agglomerative} (or \emph{divisive} for
  \code{diana} objects) coefficient as defined by Kaufman and Rousseeuw,
  see \code{\link{agnes.object} $ ac} or \code{\link{diana.object} $ dc}.
}
\examples{
data(agriculture)
aa <- agnes(agriculture)
coef(aa) # really just extracts aa$ac
coef(as.hclust(aa))# recomputes
\dontshow{
 d.a <- dist(agriculture, "manhattan")
 for (m in c("average", "single", "complete")) 
    stopifnot(all.equal(coef(hclust(d.a, method=m)),
                        coef(agnes (d.a, method=m)), tol=1e-13))
}
}
\keyword{cluster}