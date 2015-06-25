# osrmint
PostgreSQL route OSRM integration

Para compilar/instalar:

```bash
git clone https://github.com/fpacheco/osrmint.git
mkdir build
cd build
cmake ..
make
cd ..
sudo ./instalar
```

Para tests se crea una base de datos específica:

```bash
cd sql
./runtests
```
