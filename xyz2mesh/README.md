# Install Dependences


## Install Eigen

1. Download [Eigen source code](http://eigen.tuxfamily.org/index.php?title=Main_Page)

2. Un Zip it

3. Move the subdirectory ```Eigen/``` to your ```/usr/local/include```

## Install CGAL

1. Download [CGAL source code](http://www.cgal.org/)

2. Un zip it

3. Compile

	cd CGAL-4.5 # go to CGAL directory
	cmake . # configure CGAL
	make # build the CGAL libraries

4. Install

	make install

#  Compile xyz2mesh

cd LIDar-tools/xyz2mesh/
cgal_create_CMakeLists -s xyz2mesh 
cmake -DCGAL_DIR=/usr/local/include/CGAL . # Double check this!!
make