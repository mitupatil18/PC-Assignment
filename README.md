# PC-Assignment
1. Install Necessary libraraies :
   a. !apt-get -qq install -y libopencv-dev
   b. !apt-get install pkg-config
2. !g++ -fopenacc -o encodeusingopenacc encodeusingopenacc.cpp $(pkg-config --cflags --libs opencv4)
3. !./encodeusingopenacc
