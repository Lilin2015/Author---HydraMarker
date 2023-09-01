# **HydraMarker**

This is the demo code of the paper (under review), "HydraMarker: Effificient, Flexible, and Multifold Marker Field Generation"
## Introduction
An $n$-order **marker field** is a special binary matrix whose $n\times n$ sub-matrices are all distinct from each other in four orientations. It can be regarded as "**aperiodic 4-orientable de Bruijn torus**". For example, the following matrix is a 3-order marker field (black/white corresponds to 0/1 repectively).
<br /><img src=https://github.com/Lilin2015/Author---HydraMarker/blob/main/README_md_files/field.jpeg width=200><br />
You cannot find two identical 3 $\times$ 3 sub-regions from this matrix, even with rotation. Every 3 $\times$ 3 sub-region is an **ID tag** of this marker field. The 3 $\times$ 3 mask used to extract ID tag is named **tag shape**.
With this toolkit, we can go further. For example, the following matrix is a 60 $\times$ 60 marker field with 4 tag shapes, including 4 $\times$ 4, 3 $\times$ 6, 2 $\times$ 9, and 1 $\times$ 20. Try it.<br />
<img src=https://github.com/Lilin2015/Author---HydraMarker/blob/main/README_md_files/field2.png width=200><br />
Why we need marker field? One of the reasons is for **position-sensing marker**. Imagine a remote control vehicle sliding on the marker, and locating itself by a bottom view camera, which has a very limited field-of-view, as shown below. The vehicle can still estimate its position on this marker, because the sub-region it observes is unique.<br />
<img src=https://github.com/Lilin2015/Author---HydraMarker/blob/main/README_md_files/ps_marker.png width=200><br />
A commercial example is the camera-based digital pen. The pen captures and converts handwritten analog information into digital data. It locates itself on a digital paper based on a bottom view camera. The camera observes the dot array on the paper, but only a small sub-region (typically 6 $\times$ 6). The pen must read its absolute position on this paper based on this 6 $\times$ 6 ID tag. See the trick?
<br />
<img src=https://github.com/Lilin2015/Author---HydraMarker/blob/main/README_md_files/digital_pen.png width=200><br />

However, generating marker field is not easy. The most practical method was proposed by Szentandrasi *et al*.

    I. Szentandrasi, M. Zacharias, J. Havel, A. Herout, M. Dubska, and R. Kajan, “Uniform marker fifields: Camera localization by
    orientable de bruijn tori,” in Proc. IEEE International Symposium on Mixed and Augmented Reality. IEEE, 2012, pp. 319–320.
 You can download some ready-made marker fields from their project page, http://www.fit.vutbr.cz/research/groups/graph/MF/.
 However, if you want to customize your marker fields, do something more than its original definition, but do not have a supercomputer, and do not know how to modify their genetic algorithm, this HydraMarker toolkit is your best choice (at least for now).

## Instructions of generation algorithm
There are two versions of HydraMarker.
The **C++ & OpenCV version** is super fast. It can finish a 10 $\times$ 10 3-order marker field in 0.03 seconds, and a 85 $\times$ 85 4-order marker field in 1.5 minutes. The paper uses this version for experiments.
The **Matlab version** is built earlier. It is significantly slower, and the method is slightly different. But if Matlab is more convenience to you, it is totally OK. It can finish a 85 $\times$ 85 4-order marker field in 1 hour, and the difference is trivial (we will update the Matlab version shortly).<br /><br />
There is an **example.cpp/example.m** in C++ & OpenCV / Matlab version. It will teach you how to define marker field and tag shapes. The relevant functions are also richly commented.
<br /><br />
Note that, you can highly customize your marker field by presetting some values. There are four states, 0, 1, $u$, and $h$,
where $u$ means unknown and $h$ means hollow. The toolkit will fill the unknowns and ignore the hollows. You can also define multiple tag shapes, and the toolkit will try to build a marker field satisfying them simultaneously.
<br /><br />
For example, if you want a 30 $\times$ 30 circular marker field, whose center area is hollow, and contains a black cross. The input marker field should be drawn as below, where black/red/gray means 0/ $u$ / $h$ respectively.

<br /><img src=https://github.com/Lilin2015/Author---HydraMarker/blob/main/README_md_files/init.png width=200><br />

Your camera scope can cover a sub-region about 4 $\times$ 4 size, thus the first tag shape should be a 4 $\times$ 4 matrix full of 1. Sometimes, the line of sight is tilt, leading to a narrow scope, thus the second tag shape can be a 2 $\times$ 8 matrix full of 1. For some weird reason, your camera might have a biased cross shape scope, thus the third tag shape can be the matrix shown below, where black/white means 0/1 respectively (this might be too weird, but this is just an example to show what you can do for customization).

<br />
 <img src=https://github.com/Lilin2015/Author---HydraMarker/blob/main/README_md_files/shape3.png width=200><br />
 
 Then, feed the initial marker field and tag shapes to the toolkit, and start the generation process. You might get a marker field like this.

<br /> <img src=https://github.com/Lilin2015/Author---HydraMarker/blob/main/README_md_files/suppose.png width=200>
<br />

More examples below. From left to right:
1. A 90 × 90 marker field with 4 × 4 tag shape and three
empty corners. These corners can be filled by other functional patterns, such as QR locators.
2. A 123 × 123 marker field with 5 × 5 tag shape. An array
of locators are inserted. Different from the QR locators,
these insertions are part of the marker field, as red and
green correspond to 0 and 1 respectively.
3. An 85 × 85 frame-shape marker field. The tag shapes
are 4 × 4, 3 × 6, 2 × 9, and 1 × 20.
<br /> <br />
<img src=https://github.com/Lilin2015/Author---HydraMarker/blob/main/README_md_files/Ad3.png width=200> <img src=https://github.com/Lilin2015/Author---HydraMarker/blob/main/README_md_files/Ad4.png width=200> <img src=https://github.com/Lilin2015/Author---HydraMarker/blob/main/README_md_files/Ad6.png width=200>
## Instructions of reading algorithm
How to draw a position-sensing marker based on marker fields? And how to read ID tags from the position-sensing marker? It depends on your application, and there is no standard answer. Actually, this is the world we would like to open for you with this toolkit. It is your responsibility from now on. 
<br /><br />
However, if you have no idea yet, let us give you another short ride. <br /><br />There is an example. We draw a chessboard, and put dots guided by a marker field, as shown below.
<br /><img src=https://github.com/Lilin2015/Author---HydraMarker/blob/main/README_md_files/trans_markerfield.png width=100>
<img src=https://github.com/Lilin2015/Author---HydraMarker/blob/main/README_md_files/trans_chessboard.png width=100><br />
Then, we print this marker on a paper, attach it on a tool for tracking, and read the marker based on the demo code "Example of Reading Algorithm".
The reading process has 3 steps: **detect**, **construct**, **recognize**.
The "detect" step detects cross points in an image (the 1st figure below); The "construct" step organizes the detected cross points into grids to recover the background chessboard (the 2nd figure below); The "recognize" step checks whether each chessboard square has a dot, and compare the results with the marker field to determine the index of each organized cross point (the 3rd figure below).  Now, every cross point on this curved surface has been identified. We can do the tracking by PnP and BA based on the model of this tool.
<br />
<img src=https://github.com/Lilin2015/Author---HydraMarker/blob/main/README_md_files/detect.png width=200> <img src=https://github.com/Lilin2015/Author---HydraMarker/blob/main/README_md_files/construct.png width=200> <img src=https://github.com/Lilin2015/Author---HydraMarker/blob/main/README_md_files/track.png width=200><br />
This is a rough example. We believe you will have better ideas of utilizing marker fields. Feel free to use this toolkit! If convenient, please cite our paper (it is not ready yet) or this address.
