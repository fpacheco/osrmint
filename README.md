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

Para tests se crea una base de datos específica. Para su ejecución es necesario python y pydal (pip install pydal):

```bash
cd tests/python
./run_tests
```

Por mas información consulte la Wiki: https://github.com/fpacheco/osrmint/wiki.
