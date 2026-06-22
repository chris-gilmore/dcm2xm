# dcm2xm

Reads and interprets dcm assets from N64's The New Tetris and converts them to xm files.  This is not a full reconstruction of the original xm files created by Neil D. Voss, since a dcm asset is processed xm pattern data.  This tool can re-create the 16 songs from the game (as partially reconstructed xm files), which can then be played from most media players and trackers.

## Make `dcm2xm`
```
$ mkdir ~/src
$ cd ~/src
$ git clone https://github.com/chris-gilmore/dcm2xm.git
$ cd dcm2xm
$ make
```

## Extract samples and dcm assets from ROM

Place `tnt.z64` under `~/`.
```
$ crc32 tnt.z64
528a07fa

$ md5sum tnt.z64
7a28179b00734c9aa0f0609fafaafd5f

$ sha1sum tnt.z64
83fff25e82181a6993f28c91b9eeb8430396838b
```

### Install dependencies
Redhat-based
```
$ sudo dnf install lzo-devel python3-devel
$ pip3 install python-lzo
```
Debian-based
```
$ sudo apt-get install python3-lzo
```

### Download `n64tetristools`
```
$ cd ~/src
$ git clone https://github.com/chris-gilmore/n64tetristools.git
```

### Extract all samples
```
$ cd ~/src/dcm2xm
$ mkdir sample
$ cd sample
$ ~/src/n64tetristools/tnt-extract.py -v ~/tnt.z64 --all-samples
```

### Extract all dcm assets
```
$ cd ~/src/dcm2xm
$ mkdir dcm
$ cd dcm
$ ~/src/n64tetristools/tnt-extract.py -v ~/tnt.z64 --all-dcms
```

## Usage
```
$ ./build/dcm2xm 0 > 0.stdout  # Creates Title.xm
$ ./build/dcm2xm 1 > 1.stdout  # Creates Morocco.xm
$ ./build/dcm2xm 2 > 2.stdout  # Creates Dvie.xm
$ ./build/dcm2xm 3 > 3.stdout  # Creates Polyasia.xm
$ ./build/dcm2xm 4 > 4.stdout  # Creates Floppy.xm
$ ./build/dcm2xm 5 > 5.stdout  # Creates Pyramid.xm
$ ./build/dcm2xm 6 > 6.stdout  # Creates Giali.xm
$ ./build/dcm2xm 7 > 7.stdout  # Creates Thread6.xm
$ ./build/dcm2xm 8 > 8.stdout  # Creates Haluci.xm
$ ./build/dcm2xm 9 > 9.stdout  # Creates Mayan.xm
$ ./build/dcm2xm 10 > 10.stdout  # Creates Greek.xm
$ ./build/dcm2xm 11 > 11.stdout  # Creates Egypt.xm
$ ./build/dcm2xm 12 > 12.stdout  # Creates Celtic.xm
$ ./build/dcm2xm 13 > 13.stdout  # Creates Africa.xm
$ ./build/dcm2xm 14 > 14.stdout  # Creates Japan.xm
$ ./build/dcm2xm 15 > 15.stdout  # Creates Kalinka.xm
```
