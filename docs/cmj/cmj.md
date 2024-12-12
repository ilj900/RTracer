In the [cmj.pdf](https://graphics.pixar.com/library/MultiJitteredSampling/paper.pdf) you can find Pixar's explanation of how correlated multi-jittered sampling works. \
Here's how I understand it: \
`cmj(int s, int m, int n, int p)` - is the function that will be used.

*UnitSquareDescription* \
![UnitSquareDescription](./a.jpg "UnitSquareDescription")

s - defines the index of subregion that will be used to generate random sample \
n, m - the number of rows and columns that a unit square will be partitioned into \
p - kinda like a seed.

Let's see how it all works. \
What for example would happen if we call cmj with constant s = 3, m = 5 and n = 5 and varying p from 0 to 100. \

*SampledUnitSquare* \
![SampledUnitSquare](./b.bmp "SampledUnitSquare")

As you can see we sampled 100 values from 4th subregion.

To see the advantage of stratified sampling lets compare it to a random one. \
<mark>RandomSamples</mark> shows 4096 random samples with standard mt19937 generator \
<mark>CMJSamples</mark> shows 4096 random samples with a grid size of 64, which actually places each sample in its unique cell.

*RandomSamples* \
![RandomSamples](./c.bmp "RandomSamples")

*CMJSamples* \
![CMJSamples](./d.bmp "CMJSamples")

Now we have to come up with a way to uniformly pick a subregion, so that if we have 4096 subregions, \
all of them would be picked after 4096 calls.