# Setup
The code was tested running `Ubuntu 22.04` on both WSL2 and a VMWare Virtual Machine, and using `OpenCV 4.5.4`.

To compile the code, simply run the following line from a terminal in the project's root folder:

```[bash]
mkdir build && cd build && cmake .. && make
```

# Examples

The first example focuses on overlaying the patch, which is a static image, on a sample video:
```[bash]
./basicar ../data/original.jpg ../data/video.mp4 --patch=../data/patch.jpg
```
![ex1](/home/tensai/Documents/dev/task04/src/data/screenshots/screenshot11-04-2023 23-55-22.jpg)

The second example is very similar, but the overlayed asset is another video:


```[bash]
./basicar ../data/original.jpg ../data/video.mp4 --patch=../data/patch.jpg --video2=../data/video2.mp4
```

![ex2](/home/tensai/Documents/dev/task04/src/data/screenshots/screenshot12-04-2023 01-18-33.jpg)

# Features

- Pressing `S` while running the project will save a screenshot into `src/build/data/screenshots`. The exact name and path will also be logged in the console.
- Pressing `Esc` or `Q` while running the project will terminate its execution.
- When the execution ends, the rendered video can be found in `src/build/data/output.avi`.

# Rendering Simbad with OVIS

to-do
