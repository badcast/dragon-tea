# Dragon-tea
Dragon Tea. Free Messenger

## Dependency (minimum required): 

* ```libcurl >=8.0.0``` - HTTP I/O request-responce (as default). 
* ```gtk3 >= 1.3.20``` - User Interface.
* ```json-c >= 0.16``` - Uses as JSON parser.  
* ```glib >= 2.76 ``` - Uses for Structures, and controls internal system (callbacks, async operation, array, .etc). 

## How to build? Build in 5 steps!


#### Step 1. Cloning repository to dragon-tea (current directory)
```bash 
git clone "https://github.com/badcast/dragon-tea.git" "./dragon-tea"
```

#### Step 2. Generate CMakeFiles with make (optional use ninja), next configure with Release (optimization)
```bash 
cmake -S ./dragon-tea -B ./build-dragon-tea -DCMAKE_BUILD_TYPE=Release
```

#### Step 3. Compilling (optional use -j $(nproc) for maximum parallel of cores)
```bash
cmake --build ./build-dragon-tea -j $(nproc)
```

#### Step 4. Set the exec permission. 
```bash
chmod +x ./build-dragon-tea/msg-app
```

#### Step 5. Launch and Enjoy!
```bash
./build-dragon-tea/msg-app
```
