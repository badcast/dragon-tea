# Dragon-tea
Dragon Tea. Free Messenger

## Build it yourself
-----

| Build dependencies    | Dependencies           | Description                                                                                  |
|-----------------------|------------------------|----------------------------------------------------------------------------------------------|
| gtk3-dev >= 1.3.20    | gtk3 self depend.      | User Interface.                                                                              |
| glib >= 2.76          | glib self depend.      | Uses for structures, controls, callbacks, async operation, array, etc.                       |
| libcurl >= 8.0.0      | libcurl self depend.   | HTTP I/O request-responce (as default).                                                      |
| json-c >= 0.16        | libjson-c self depend. | Uses as JSON parser.                                                                         |
| cmake                 |                        |                                                                                              |
| pkg-config            |                        |                                                                                              |


## How to build? Build in 5 steps!
-----

#### Step 1. Cloning repository to dragon-tea (current directory)

    git clone "https://github.com/badcast/dragon-tea.git" "./dragon-tea"


#### Step 2. Generate CMakeFiles with make (optional use ninja), next configure with Release (optimization)

    cmake -S ./dragon-tea -B ./build-dragon-tea -DCMAKE_BUILD_TYPE=Release


#### Step 3. Compilling (optional use -j $(nproc) for maximum parallel of cores)

    cmake --build ./build-dragon-tea -j $(nproc)


#### Step 4. Set the exec permission. 

    chmod +x ./build-dragon-tea/msg-app


#### Step 5. Launch and Enjoy! 

    ./build-dragon-tea/msg-app