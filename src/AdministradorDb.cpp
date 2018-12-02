/*
 * Copyright (c) 2018 Los Autores del Proyecto <AUTORES.md>
 *
 * Se concede permiso, libre de cargos, a cualquier persona que obtenga una
 * copia de este software y de los archivos de documentación asociados
 * (el "Software"),para utilizar el Software sin restricción, incluyendo sin
 * limitación losderechos a usar, copiar, modificar, fusionar, publicar,
 * distribuir, sublicenciar, y/o vender copias del Software, y a permitir a las
 * personas a las que se les proporcione el Software a hacer lo mismo, sujeto a
 * las siguientes condiciones:
 *
 * El aviso de copyright anterior y este aviso de permiso se incluirán en todas
 * las copias o partes sustanciales del Software.
 *
 * EL SOFTWARE SE PROPORCIONA "TAL CUAL", SIN GARANTÍA DE NINGÚN TIPO, EXPRESA O
 * IMPLÍCITA, INCLUYENDO PERO NO LIMITADA A GARANTÍAS DE COMERCIALIZACIÓN,
 * IDONEIDAD PARA UN PROPÓSITO PARTICULAR Y NO INFRACCIÓN. EN NINGÚN CASO LOS
 * AUTORES O PROPIETARIOS DE LOS DERECHOS DE AUTOR SERÁN RESPONSABLES DE NINGUNA
 * RECLAMACIÓN, DAÑOS U OTRAS RESPONSABILIDADES, YA SEA EN UNA ACCIÓN DE
 * CONTRATO, AGRAVIO O CUALQUIER OTRO MOTIVO, DERIVADAS DE, FUERA DE O EN
 * CONEXIÓN CON EL SOFTWARE O SU USO U OTRO TIPO DE ACCIONES EN EL SOFTWARE.
 */

#include "AdministradorDb.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>

/**
 * Apuntador a la única instancia de está clase
 */
static AdministradorDb* INSTANCE = Q_NULLPTR;

/**
 * Cierra la base de datos antes de que la clase
 * sea destruída
 */
AdministradorDb::~AdministradorDb() {
    cerrarBaseDeDatos();
}

/**
 * Regresa la única instancia de la clase.
 * El administrador de la base de datos debe de tener
 * una sola instancia para evitar errores al intentar
 * abrir o manejar la base de datos.
 */
AdministradorDb* AdministradorDb::instancia() {
    if (INSTANCE == Q_NULLPTR)
        INSTANCE = new AdministradorDb();

    return INSTANCE;
}

/**
 * Regresa una lista con los nombres completos de los profesores activos
 */
QStringList AdministradorDb::profesores() {
    QStringList profesores;
    QStringList campos = {"ID", "Apellido Paterno", "Apellido Materno", "Nombres"};
    QList<QList<QVariant>> consulta = ejecutarConsulta(campos, "Datos Personales");

    // Validar resultados
    if (consulta.length() == campos.length()) {
        // Obtener lista de apellidos y nombres
        QList<QVariant> ids = consulta.at (0);
        QList<QVariant> apellidosPaternos = consulta.at(1);
        QList<QVariant> apellidosMaternos = consulta.at(2);
        QList<QVariant> nombres = consulta.at(3);

        // Solo registrar el profesor si las listas de apellidos tiene
        // el mismo numero de elementos que la lista de nombres
        if (nombres.length() == apellidosMaternos.length() &&
            nombres.length() == apellidosPaternos.length()) {
            // Registrar nombre de cada profesor (mientras esten marcados
            // como profesores activos en la institucion)
            for (int i = 0; i < apellidosPaternos.length(); ++i) {
                // Checar si profesor sigue activo en la institucion
                if (leerDato (ids.at (i).toInt(), "Datos Generales", "Inactivo") == "true")
                    continue;

                // Registrar nombre
                profesores.append(tr("%1 %2 %3")
                                  .arg(apellidosPaternos.at(i).toString())
                                  .arg(apellidosMaternos.at(i).toString())
                                  .arg(nombres.at(i).toString()));
            }
        }
    }

    // Regresar lista de profesores
    return profesores;
}

/**
 * @brief AdministradorDb::leerDato
 * @param id
 * @param tabla
 * @param identificador
 * @return
 */
QString AdministradorDb::leerDato (const int id,
                                   const QString &tabla,
                                   const QString &identificador) {
    // Terminar funcion si la base de datos no esta abierta y configurada
    // o la identificacion del profesor es invalida
    if (!disponible() || id < 0)
        return "";

    // Generar commando
    QString commando = tr("SELECT [%1] FROM [%2] WHERE ID=%3")
            .arg(identificador)
            .arg(tabla)
            .arg(id);

    // Ejecutar consulta
    QSqlQuery query;
    if (query.exec(commando) && query.next())
        return query.value(0).toString();

    // Regresar valor vacio
    return "";
}


bool AdministradorDb::escribirDato (const int id,
                                    const QString &tabla,
                                    const QString &identificador,
                                    const QString &valor) {
    return false;
}


/**
 * Registra un nuevo profesor en la base de datos y regresa
 * el objeto que permite modificar/leer los datos del nuevo
 * profesor.
 */
int AdministradorDb::registrarProfesor() {
    // Solo intentar registrar el profesor si estamos conectados
    // a la base de datos
    if (disponible()) {
        return 0;
    }

    // Hubo un error al intentar registrar el nuevo profesor
    return -1;
}

/**
 * Regresa @c true si existe una conexión con la base de datos
 */
bool AdministradorDb::disponible() {
    return m_database.isOpen();
}

/**
 * Regresa un apuntador al objeto que administra la base de datos
 */
QSqlDatabase& AdministradorDb::baseDeDatos() {
    return m_database;
}

/**
 * Regresa la ubicación de la base de datos, usada para cambiar
 * el titulo de la ventana
 */
QString AdministradorDb::ubicacionBaseDeDatos() const {
    return m_dbUbicacion;
}

/**
 * @brief AdministradorDb::acercaDeQt
 */
void AdministradorDb::acercaDeQt() {
    QApplication::aboutQt();
}

/**
 * Crea una nueva base de datos en una ubicacion definida
 * por el usuario y la abre
 */
void AdministradorDb::nuevaBaseDeDatos() {
    // Obtener ubicacion para nueva base de datos
    QString db = QFileDialog::getSaveFileName(Q_NULLPTR,
                                              tr("Crear base de datos vacía"),
                                              QDir::homePath(),
                                              tr("Bases de Datos de Access (*.mdb *.accdb)"));

    // Guardar template de base de datos vacía
    if (!db.isEmpty()) {
        // Escribir datos de plantilla a base de datos de salida
        QFile out(db);
        if (out.open(QFile::WriteOnly)) {
            QFile plantilla(":/db/plantilla.mdb");
            if (plantilla.open(QFile::ReadOnly)) {
                if (out.write(plantilla.readAll()) == plantilla.size()) {
                    // Cerrar archivos
                    out.close();
                    plantilla.close();

                    // Abrir base de datos
                    configurarBaseDeDatos(db);

                    // Mostrar mensaje de exito
                    mostrarInfo(tr("Información"),
                                tr("La nueva base de datos fue generada exitosamente!"));

                    // Terminar ejecucion de funcion
                    return;
                }
            }
        }

        // Registrar error
        mostrarError(tr("Error"),
                     tr("Error al crear la nueva base de datos!"));
    }
}

/**
 * Pide al usuario que seleccione una base de datos.
 * Posteriormente, se intenta configurar una conexión a la
 * base de datos seleccionada por el usuario.
 */
void AdministradorDb::abrirBaseDeDatos() {
    // Obtener archivo seleccionado por el usuario
    QString db = QFileDialog::getOpenFileName(Q_NULLPTR,
                                              tr("Seleccionar base de datos"),
                                              QDir::homePath(),
                                              tr("Bases de Datos de Access (*.mdb *.accdb)"));

    // Intentar abrir la base de datos
    if (!db.isEmpty())
        configurarBaseDeDatos(db);
}

/**
 * Termina la conexión con la base de datos actual
 */
void AdministradorDb::cerrarBaseDeDatos() {
    // Checar que la base de datos este abierta
    if (disponible()) {
        // Obtener nombre de conexion (para quitarla despues de cerrar DB)
        QString conexion = baseDeDatos().connectionName();

        // Cerrar DB
        baseDeDatos().close();

        // Quitar conexion y registrar DB vacia
        QSqlDatabase::removeDatabase(conexion);
        m_database = QSqlDatabase();
        m_dbUbicacion = "";

        // Notificar UI
        emit baseDeDatosCambiada();
    }
}

void AdministradorDb::mostrarEstadisticas() {

}

/**
 * Cambia el estado de registro a inactivo del profesor con la ID especificada.
 * Si @a silent es @c true, no se va a mostrar un mensaje de retroalimentacion
 * al usuario.
 */
void AdministradorDb::eliminarProfesor (const int id, const bool silent) {
    if (!disponible())
        mostrarError(tr("Error"),
                     tr("No hay ninguna base de datos cargada por la aplicación!"));

    else {
    }
}

void AdministradorDb::mostrarInfo (const QString &titulo, const QString &texto) {
    QMessageBox::information(Q_NULLPTR, titulo, texto);
}

void AdministradorDb::mostrarError (const QString &titulo, const QString &texto) {
    QMessageBox::warning(Q_NULLPTR, titulo, texto);
}

/**
 * Intenta establecer una conexión con la base de datos en la @a ubicacion
 * definida. Si hay un error, entonces se va a mostrar una notificacion al
 * usuario.
 */
void AdministradorDb::configurarBaseDeDatos(const QString& ubicacion) {
    // Checar que la ubicacion no este vacía
    if (ubicacion.isEmpty()) {
        mostrarError (tr("Error"),
                      tr("La ubicación de la base de datos no puede estar vacía!"));
        return;
    }

    // Checar que la base de datos existe
    QFileInfo info(ubicacion);
    if (!info.exists() || !info.isFile()) {
        mostrarError(tr("Error"),
                     tr("El archivo \"%1\" no existe!"));
        return;
    }

    // Cerrar base de datos existente
    cerrarBaseDeDatos();

    // Crear DBQ
    QString dbq = ubicacion;

    // Abrir la nueva base de datos
    m_database = QSqlDatabase::addDatabase("QODBC");

    // Implementaciones para cada SO
#if defined (Q_OS_WIN)
    dbq.replace(QChar('/'), "\\");
    m_database.setDatabaseName("Driver={Microsoft Access Driver (*.mdb, *.accdb)};"
                               "DSN='';DBQ=" + dbq + ";");
#elif defined (Q_OS_LINUX)
    m_database.setDatabaseName("Driver=MDBTools;DBQ='" + dbq + "';");
#else
    mostrarError(tr("Error"),
                 tr("Este sistema operativo no está soportado por la "
                    "aplicación!"));
#endif

    // Notificar al usuario si hubo un error
    if (!m_database.open()) {
        QMessageBox message;
        message.setWindowTitle(tr("Error"));
        message.setIcon(QMessageBox::Critical);
        message.setText("<h3>" + tr("Error abriendo la base de datos") + ":</h3>");
        message.setInformativeText(m_database.lastError().text());
        message.setStandardButtons(QMessageBox::Ok);
        message.exec();
        cerrarBaseDeDatos();
    }

    // Actualizar ubicacion de DB
    else
        m_dbUbicacion = ubicacion;

    // Actualizar estado de la aplicacion
    emit baseDeDatosCambiada();
}

/**
 * @brief AdministradorDb::ejecutarConsulta
 * @param campos
 * @param tabla
 *
 * Ejecuta una lectura de la base de datos con una lista de @a campos en la
 * @a tabla definida. Se hacen ajustes para soportar campos y tablas con
 * espacios.
 *
 * @return
 */
QList<QList<QVariant>> AdministradorDb::ejecutarConsulta(const QStringList campos,
                                                         const QString tabla) {
    // Inicializar lista
    QList<QList<QVariant>> resultados;

    // Terminar funcion si la base de datos no esta conectado
    if (!disponible())
        return resultados;

    // Hacer un query individual para cada campo y registrar los resultados
    // en una matriz ("lista de listas")
    for (int i = 0; i < campos.length(); ++i) {
        QList<QVariant> resultadosCampo;

        // Inicializar commando de SELECT
        QString commando;
        commando.append ("SELECT [");
        commando.append (campos.at (i));

        // Generar seccion de tabla
        commando.append ("] FROM [");
        commando.append(tabla);
        commando.append("]");

        // Ejecutar query
        QSqlQuery query;
        query.exec(commando);
        while (query.next())
            resultadosCampo.append(query.value(0));

        // Agregar resultados de query
        resultados.append (resultadosCampo);
    }

    // Regresar resultados
    return resultados;
}
