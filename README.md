# Turtle POP3 Server

|Alumno|Legajo|Mail|
|-|-|-|
|Alejo Flores Lucey|62622|afloreslucey@itba.edu.ar|
|Andrés Carro Wetzel|61655|acarro@itba.edu.ar
|Nehuén Gabriel Llanos|62511|nllanos@itba.edu.ar

## Compilación y creación de los ejecutables

Para la creación de los ejecutables necesarios del proyecto basta con posicionarse en la carpeta raíz del mismo y ejecutar el comando `make all`. De esta manera se crearán dos ejecutables, `turtle-client` y `turtle-pop3` en la carpeta `./bin`.

```
$$ make all
```

## Utilización del directorio de mails

Para utilizar el servidor POP3, se requiere de un directorio que contenga todos los correos de los usuarios del sistema. Este directorio debe contener como hijos otros directorios con los nombres de usuario. Dentro de cada uno de esos directorios debe existir una carpeta llamada `cur`, que contendrá los correos, cada uno en un archivo separado.

El esquema siguiente muestra cómo debe quedar todo, suponiendo que el directorio del mail se llama `mail` y el servidor tiene tres usuarios (arnold, rocky, ronnie):

```
.
└── mail
    ├── arnold
    │   └── cur
    │       ├── mail1
    │       ├── mail2
    │       └── mail3
    ├── rocky
    │   └── cur
    │       ├── mail1
    │       └── mail2
    └── rocky
        └── cur
            ├── mail1
            ├── mail2
            ├── mail3
            ├── mail4
            ├── mail5
            └── mail6
```

## Ejecución de las aplicaciones servidor y cliente

Una vez creado los ejecutables y los directorios, procedemos a ejecutar el servidor. Nos posicionamos en la carpeta `bin` que se encuentra en la raíz del proyecto.

### Ejecución del servidor

Iniciado por el servidor corremos lo siguiente para poder obtener información de los diferentes argumentos que les podemos pasar:

```
$$ ./turtle-pop3 -h
```

La lista completa de parámetros que acepta el servidor es el siguiente:

- `--help` ó `-h`: Imprime un mensaje de ayuda.
- `--directory <maildir>` ó `-d <maildir>`: Especificar el path del directorio donde se encontrarán todos los usuarios con sus mails.
- `--pop3-server-port <pop3 server port>` ó `-p <pop3 server port>`: Puerto entrante para conexiones al servidor POP3.
- `--config-server-port <configuration server port>` ó `-P <configuration server port>`: Puerto entrante para conexiones de configuración.
- `--user <user>:<password>` ó `-u <user>:<password>`: Usuario y contraseña de usuario que puede usar el servidor POP3. Hasta 10.
- `--token <token>` ó `-t <token>`: Token de autenticación para el cliente.
- `--version` ó `-v` Imprime información sobre la versión.

A modo de ejemplo, se provee el siguiente comando para iniciar el servidor

```bash
$$ ./turtle-pop3 -p 61655 -u arnold:sch -t arnold -d ../../mail
```

El servidor quedará esperando conexiones TCP en el puerto especificado. Quien se conecte podrá utilizar el servidor POP3.

### Ejecución del cliente

Por otro lado, se puede ejecutar la aplicación cliente, que permite hacer modificaciones y obtener información del servidor POP3. Nos posicionamos en la carpeta `bin` dentro de la raíz del proyecto y corremos el siguiente comando:

```bash
$$ ./turtle-client -h
```

La lista completa de parámetros que acepta el cliente es el siguiente:

- `--help` ó `-h`: Imprime un mensaje de ayuda.
- `--token <token>` ó `-t <token>`: Token de autenticación para el cliente.
- `--port <server port>` ó `-P <server port>`: Puerto para conexiones al servidor POP3 a administrar.
- `--directory <maildir>` ó `-d <maildir>`: Path del directorio donde se encotrarán todos los usuarios con sus mails.
- `--add-user <user>:<password>` ó `-u <user>:<password>`: Usuario y contraseña de usuario que puede usar el servidor POP3. Hasta 10.
- `--change-password <user>:<password>` ó `-c <user>:<password>`: Cambiar contraseña para el usuario especificado.
- `--remove-user <user>` ó `-r <user>`: Eliminar usuario del servidor POP3.
- `--list-users` ó `-l`: Listar los usuarios del servidor POP3.
- `--statistics` ó `-s`: Obtener las estadísticas del servidor POP3.
- `--max-mails <number>` ó `-m <number>`: Cambiar el máximo número de mails.
- `--version` ó `-v`: Imprime información sobre la versión.

A modo de ejemplo, con el siguiente comando se obtendrán las estadísticas del servidor.

```bash
$$ ./turtle-client -P 62622 -t arnold -s
```

Por último, si quiere ver los logs, debe ir a la carpeta `bin` presente en la carpeta raíz del proyecto y ahí existirá un archivo denominado `turtle-pop3.log`.

## Aclaraciones importantes

* El token de autenticación en el servidor y el cliente **DEBE** ser el mismo si se quiere que el cliente pueda modificar o obtener información del servidor

* El puerto por defecto del servidor POP3, que puede ser cambiado con el argumento `-p`, es 61655

* El puerto por defecto para que el cliente y el servidor se comuniquen, que puede ser cambiado con el argumento `-P` en ambos ejecutables, es 62622

