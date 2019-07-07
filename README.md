### Build

see CMakeList.txt
Boost 1.70.0, OpenSSL, nlohmann_json(https://github.com/nlohmann/json) ozo(submoduled)

`./rebuild.sh`


### Run

```
cd build
./niba-server /path/to/config # or just ./niba-server
cat ../testscript - | ./niba-client # or just ./niba-client
```


### Config

```
{
    "host": "0.0.0.0",
    "port": 19999,
    "threads": 1,
    "static_conn_str": "dbname=niba_static user=postgres",
    "player_conn_str": "dbname=niba user=postgres"
}
```