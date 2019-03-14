### Build

see CMakeList.txt
Boost 1.68.0, OpenSSL, nlohmann_json(https://github.com/nlohmann/json)
ozo(submoduled)
`./rebuild.sh`


### Run

```
cd build
./niba-server /path/to/config # or just ./niba-server
./niba-client ../testscript # or just ./niba-client or cat ../testscript - | ./niba-client
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