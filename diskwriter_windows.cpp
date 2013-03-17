#include "diskwriter_windows.h"

#include "zlib.h"

#include <QDebug>
#include <QApplication>

DiskWriter_windows::DiskWriter_windows(QObject *parent) :
    DiskWriter(parent)
{
}

DiskWriter_windows::~DiskWriter_windows()
{
    if (dev.isOpen()) {
        dev.close();
    }
}

int DiskWriter_windows::open(const QString &device)
{
    dev.setFileName(device);
    if (!dev.open(QFile::WriteOnly)) {
        return -1;
    }
    return 0;
}

void DiskWriter_windows::close()
{
    dev.close();
}

bool DiskWriter_windows::isOpen()
{
    return dev.isOpen();
}

bool DiskWriter_windows::writeCompressedImageToRemovableDevice(const QString &filename)
{
    int r;
    bool ok;
    // 512 == common sector size
    char buf[512*1024];

    if (!dev.isOpen()) {
        qDebug() << "Device not ready or whatever";
        return false;
    }

    // Open source
    gzFile src = gzopen(filename.toStdString().c_str(), "rb");
    if (src == NULL) {
        qDebug() << "Couldn't open file:" << filename;
        return false;
    }

    if (gzbuffer(src, 128*1024) != 0) {
        qDebug() << "Failed to set buffer size";
        gzclose_r(src);
        return false;
    }

    r = gzread(src, buf, sizeof(buf));
    while (r > 0) {
        // TODO: Sanity check
        ok = dev.write(buf, r);
        if (!ok) {
            qDebug() << "Error writing";
            return false;
        }
        emit bytesWritten(gztell(src));
        QApplication::processEvents();
        r = gzread(src, buf, sizeof(buf));
    }

    if (r < 0) {
        qDebug() << "Error reading file!";
        gzclose_r(src);
        return false;
    }

    dev.close();
    gzclose_r(src);
    return true;
}