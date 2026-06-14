
## Install the standalone Asio development package

```bash
sudo apt-get update
sudo apt-get install libasio-dev
```

## Install the compression and the SSL libraries

To enable HTTP compression (Zlib) and HTTPS/SSL support (OpenSSL) in your C++ Crow application, you need to install both the runtime libraries and their corresponding development headers (-dev packages). The headers are what allow your compiler to find #include <zlib.h> or OpenSSL components during the build phase.

```bash
sudo apt update
sudo apt install zlib1g-dev libssl-dev -y
```

## Installing Crow from source

```bash
git clone https://github.com/CrowCpp/Crow.git
```

Run `mkdir build` inside of crow's source directory.

```bash
cd Crow
mkdir build
```

Navigate to the new `build` directory and run the following:

```bash
cd build
cmake .. -DCROW_BUILD_EXAMPLES=OFF -DCROW_BUILD_TESTS=OFF
```

While building you can set: the `CROW_ENABLE_SSL` variable to enable the support for https and the `CROW_ENABLE_COMPRESSION` variable to enable the support for http compression. By default, Crow sets `CROW_ENABLE_SSL=true` and `CROW_ENABLE_COMPRESSION=true` for CMake builds.

Run make install. Use `sudo` as INSTALL requires write permissions on `/usr/local/include`

```bash
sudo make install
```

Optionally, build single header `crow_all.h`

```bash
cd scripts
python3 merge_all.py ../include crow_all.h
```

`crow_all.h` is recommended only for small, possibly single source file projects, and ideally should not be installed on your system.

## Update the Makefile

You must call find_package and manually link with SSL and compression libraries in your build if you keep these defaults.

```Makefile
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Crow REQUIRED)

target_link_libraries(main
    PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
    Crow::Crow
)
```

## Build Your Project

With the core dependency installed, navigate back to your own project directory containing your CMakeLists.txt and main.cpp.

Navigate to your project directory (if separate from the ta-lib source)

```bash
cd /path/to/your/project
```

1. Configure the project and generate the build system inside the out/build directory

```bash
cmake -S . -B out/build/
```

2. Compile the project

```bash
make -C out/build/
```

## 5. Running the Application

By default, the compiled binary will be placed inside the bin/ directory relative to your project root (as defined in your CMakeLists.txt).

Execute the generated binary

```bash
./bin/test
```

## Optional: generate SSL self-signed certificate

Navigate to your project directory and run this single command:

```bash
openssl req -x509 -newkey rsa:4096 -keyout server.key -out server.crt -sha256 -days 365 -nodes -subj "/CN=localhost"
```

### What do all those flags mean?

* req -x509: Tells OpenSSL to output a self-signed certificate directly, rather than generating a Certificate Signing Request (CSR).

* newkey rsa:4096: Creates a new 4096-bit RSA cryptographic key pair.

* keyout server.key: Saves the private key file as server.key.

* out server.crt: Saves the public SSL certificate file as server.crt.

* -days 365: Sets the certificate to remain valid for exactly one year.

* nodes: Short for "No DES". This tells OpenSSL not to encrypt the private key with a password. If you omit this, your Crow server will freeze on startup every time waiting for you to type a password in the terminal.

* subj "/CN=localhost": Skips the interactive questionnaire (Country, State, Locality) and instantly binds the certificate to localhost.
