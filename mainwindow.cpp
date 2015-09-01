#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "eor_cfg.h"
#include "comboboxitemdelegate.h"
#include "crc16.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QComboBox>
#include <QDebug>
#include <QTextCodec>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    int row, col, i, j;
    QDoubleSpinBox *spin;

    ui->setupUi(this);

    ui->actionOtw_rz->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
    ui->actionZapisz->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->actionZako_cz->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton));

    ui->fileSize->setText(QString::number(eorkonf_data_size) + QString("B"));

    ui->generalTempSensorTypeCmb->addItem("TH");
    ui->generalTempSensorTypeCmb->addItem("CAN");
    ui->generalTempSensorTypeCmb->addItem("Modbus");

    ui->generalFallSensorTypeCmb->addItem("IO");
    ui->generalFallSensorTypeCmb->addItem("CAN");
    ui->generalFallSensorTypeCmb->addItem("Modbus");

    ui->sensorColdFrame->setEnabled(false);
    ui->sensorHotFrame->setEnabled(false);
    ui->blowSensorFRame->setEnabled(false);
    ui->sensorPwrCtrlFrame->setEnabled(false);

    ui->sensorColdTypeCmb->addItem("TH");
    ui->sensorColdTypeCmb->addItem("CAN");
    ui->sensorColdTypeCmb->addItem("Modbus");

    ui->sensorHotTypeCmb->addItem("TH");
    ui->sensorHotTypeCmb->addItem("CAN");
    ui->sensorHotTypeCmb->addItem("Modbus");

    ui->blowSensTypeCmb->addItem("IO");
    ui->blowSensTypeCmb->addItem("CAN");
    ui->blowSensTypeCmb->addItem("Modbus");

    row = ui->weatherAutomTempTable->rowCount();
    col = ui->weatherAutomTempTable->columnCount();

    for(i = 0; i < row; i++)
    {
        for(j = 0; j < col; j++)
        {
            spin = new QDoubleSpinBox();
            spin->setMinimum(-50.0);
            spin->setMaximum(100.0);
            spin->setDecimals(1);
            spin->setSingleStep(0.1);
            ui->weatherAutomTempTable->setCellWidget(i, j, spin);
        }
    }

    ui->weatherAutomList->addItem("Automat pogodowy 1");
    ui->weatherAutomList->setCurrentRow(0);

    strcpy(circuit_cfg[0].name, "Obwód1");
    ui->circuitList->addItem("Obwód1");
    ui->circuitList->setCurrentRow(0);

    ui->cirNameEdit->setEnabled(false);
    ui->cirActiveChk->setEnabled(false);
    ui->cirReferenceChk->setEnabled(false);
    ui->cirTypeCmb->setEnabled(false);
    ui->cirGroupNo->setEnabled(false);
    ui->cirWeatherAutomNo->setEnabled(false);
    ui->l1Frame->setEnabled(false);
    ui->l2Frame->setEnabled(false);
    ui->l3Frame->setEnabled(false);
    ui->cirRelayFrame->setEnabled(false);
    ui->cirRelayConfFrame->setEnabled(false);
    ui->referenceCirNo->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOtw_rz_triggered()
{
    QString fileNameToOpen;
    QFile file;
    QByteArray fileBuf;
    uint16_t crc16;

    if(ui->editWeatherAutomBtn->text() == "Zapisz zmiany")
    {
        QMessageBox::critical(this, "Błąd", "Zapisz zmiany w automatach pogodowych");
        return;
    }
    if(ui->editCircuitList->text() == "Zapisz zmiany")
    {
        QMessageBox::critical(this, "Błąd", "Zapisz zmiany w obwodach sterowania");
        return;
    }

    fileNameToOpen = QFileDialog::getOpenFileName(this, "Otwórz plik konfiguracji", "koneor.bin", "koneor.bin");

    if(fileNameToOpen == "")
    {
    //anuluj
    }
    else
    {
        file.setFileName(fileNameToOpen);
        if(file.open(QIODevice::ReadOnly))
        {
            fileBuf = file.readAll();
            file.close();

            if(fileBuf.size() < (eorkonf_data_size + sizeof(eorkonf_hdr_t)))
            {
                QMessageBox::critical(this, "Błąd", QString("Plik jest zbyt mały. Dane nie zostały wczytane\n") +
                                      QString("Wczytano: ") + QString::number(fileBuf.size()) +
                                      QString(" B\nOczekiwano: ") + QString::number(eorkonf_data_size + sizeof(eorkonf_hdr_t)) +
                                      QString(" B"));
                return;
            }
            else if(fileBuf.size() > (eorkonf_data_size + sizeof(eorkonf_hdr_t)))
            {
                QMessageBox::critical(this, "Błąd", QString("Plik jest za duży. Wczytane dane mogą być błędne\n") +
                                      QString("Wczytano: ") + QString::number(fileBuf.size()) +
                                      QString(" B\nOczekiwano: ") + QString::number(eorkonf_data_size + sizeof(eorkonf_hdr_t)) +
                                      QString(" B"));
            }

            setCfgStructs(fileBuf.data());
            crc16 = LiczCrc16Buf((uint8_t*)&fileBuf.data()[sizeof(eorkonf_hdr_t)], eorkonf_data_size);
            if(eorkonf_hdr.crc16 != crc16)
            {
                QMessageBox::critical(this, "Błąd", QString("Nieprawidłowe crc pliku. Wczytane dane mogą być błędne\n") +
                                      QString("Obliczone: 0x") + QString::number(crc16, 16) +
                                      QString("\nOczekiwano: 0x") + QString::number(eorkonf_hdr.crc16, 16));
            }

            if(eorkonf_hdr.file_len != eorkonf_data_size)
            {
                QMessageBox::critical(this, "Błąd", QString("Nieprawidłowy rozmiar danych w nagłówku\n") +
                                      QString("Odczytane: ") + QString::number(eorkonf_hdr.file_len) +
                                      QString(" B\nOczekiwano: ") + QString::number(eorkonf_data_size) +
                                      QString(" B"));
            }

            if(strncmp(eorkonf_hdr.id_txt, "EOR_KON", 7) != 0)
            {
                QMessageBox::critical(this, "Błąd", "Brak w nagłówku znacznika EOR_KON. Wczytane dane mogą być nieprawidłowe\n");
            }

            ui->fileSize->setText(QString::number(eorkonf_hdr.file_len + sizeof(eorkonf_hdr_t)));
            ui->fileVer->setText(QString("%1.%2").arg(eorkonf_hdr.ver).arg(eorkonf_hdr.rev));
            setGeneralCfg();
            setIOModuleCfg();
            setJsn2Cfg();
            setMeterCfg();
            setGeneralWeatherMeasure();
            setWeatherAutomCfg(0);
            setTemperaturesCfg();
            setCircuitCfg(0);
            setIoCfg();
            setModbusSlaveCfg();
            setEthCfg();
            setRsCfg();
        }
        else
        {
            QMessageBox::critical(this, "Błąd", "Wystąpił błąd podczas otwierania pliku do odczytu");
        }
    }
}

void MainWindow::on_actionZako_cz_triggered()
{
    close();
}

void MainWindow::on_actionZapisz_triggered()
{
    QString fileNameToSave;
    QFile file;
    QByteArray fileBuf;
    qint64 ret;
    char* cBuf;
    eorkonf_hdr_t *tmp_eorkonf_hdr;

    if(ui->editWeatherAutomBtn->text() == "Zapisz zmiany")
    {
        QMessageBox::critical(this, "Błąd", "Zapisz zmiany w automatach pogodowych");
        return;
    }
    if(ui->editCircuitList->text() == "Zapisz zmiany")
    {
        QMessageBox::critical(this, "Błąd", "Zapisz zmiany w obwodach sterowania");
        return;
    }

    fileNameToSave = QFileDialog::getSaveFileName(this, "Zapisz plik konfiguracji", "koneor.bin", "koneor.bin");

    if(fileNameToSave == "")
    {
    //anuluj
    }
    else
    {
        file.setFileName(fileNameToSave);
        if(file.open(QIODevice::WriteOnly))
        {
            eorkonf_hdr.ver = 1;
            eorkonf_hdr.rev = 0;
            eorkonf_hdr.file_len = eorkonf_data_size;
            strcpy(eorkonf_hdr.id_txt, "EOR_KON");
            fileBuf.append((char*)&eorkonf_hdr, sizeof(eorkonf_hdr_t));
            //todo na koniec crc16

            if(updateGeneralCfg() == false)
                QMessageBox::critical(this, "Błąd", "Błąd parametrów w części \"Podstawowe\"");

            fileBuf.append((char*)&general_cfg, sizeof(general_cfg_t));
            fileBuf.append((char*)user_cfg, sizeof(user_cfg_t) * USER_COUNT);
            if(updateIOModuleCfg() == false)
                QMessageBox::critical(this, "Błąd", "Błędna wartość w polu adres w modułach IO");

            fileBuf.append((char*)io_module_cfg, sizeof(io_module_cfg_t) * IO_MODULE_COUNT);
            if(updateJsn2Cfg() == false)
                QMessageBox::critical(this, "Błąd", "Błędna wartość w polu adres w modułach JSN-2");

            fileBuf.append((char*)jsn2_module_cfg, sizeof(jsn2_module_cfg_t) * JSN2_MODULE_COUNT);
            if(updateMeterCfg() == false)
                QMessageBox::critical(this, "Błąd", "Błędny numer seryjny licznika energii");

            fileBuf.append((char*)meter_cfg, sizeof(meter_cfg_t) * METER_COUNT);
            updateGeneralWeatherMeasure();
            fileBuf.append((char*)&general_weather_measure_cfg, sizeof(general_weather_measure_cfg_t));
            fileBuf.append((char*)weather_autom_cfg, sizeof(weather_autom_cfg_t) * WEATHER_AUTOM_COUNT);
            updateTemperaturesCfg();
            fileBuf.append((char*)&temperatures_cfg, sizeof(temperatures_cfg_t));
            fileBuf.append((char*)circuit_cfg, sizeof(circuit_cfg_t) * CIRCUIT_COUNT);
            fileBuf.append((char*)&group_cfg, sizeof(group_cfg_t));
            updateIoCfg();
            fileBuf.append((char*)&io_cfg, sizeof(io_cfg_t));
            fileBuf.append((char*)&can_cfg, sizeof(can_cfg_t));
            updateModbusSlaveCfg();
            fileBuf.append((char*)&modbus_slave_cfg, sizeof(modbus_slave_cfg_t));
            fileBuf.append((char*)&tgfm_cfg, sizeof(tgfm_cfg_t));
            updateRsCfg();
            fileBuf.append((char*)rs_cfg, sizeof(rs_cfg_t) * RS_COUNT);
            updateEthCfg();
            fileBuf.append((char*)&eth_cfg, sizeof(eth_cfg_t));

            cBuf = fileBuf.data();
            tmp_eorkonf_hdr = (eorkonf_hdr_t*)cBuf;
            tmp_eorkonf_hdr->crc16 = LiczCrc16Buf((uint8_t*)&cBuf[sizeof(eorkonf_hdr_t)], eorkonf_data_size);

            ret = file.write(fileBuf);
            if(ret == -1)
                QMessageBox::critical(this, "Błąd", QString("Błąd podczas zapisu pliku na dysk."));
            else if(ret < (eorkonf_data_size + sizeof(eorkonf_hdr_t)))
                QMessageBox::critical(this, "Błąd", QString("Błąd podczas zapisu danych na dysku\n") +
                                      QString("Zapisano: ") + QString::number(ret) +
                                      QString(" B\nPowinno być: ") + QString::number(eorkonf_data_size + sizeof(eorkonf_hdr_t)) +
                                      QString(" B"));
            file.close();
        }
        else
        {
            QMessageBox::critical(this, "Błąd", "Wystąpił błąd podczas otwierania pliku do zapisu");
        }
    }
}

void MainWindow::setCfgStructs(char* buf)
{
    int index = 0;

    memcpy(&eorkonf_hdr, &buf[index], sizeof(eorkonf_hdr_t));
    index += sizeof(eorkonf_hdr_t);

    memcpy(&general_cfg, &buf[index], sizeof(general_cfg_t));
    index += sizeof(general_cfg_t);

    memcpy(user_cfg, &buf[index], sizeof(user_cfg_t) * USER_COUNT);
    index += sizeof(user_cfg_t) * USER_COUNT;

    memcpy(io_module_cfg, &buf[index], sizeof(io_module_cfg_t) * IO_MODULE_COUNT);
    index += sizeof(io_module_cfg_t) * IO_MODULE_COUNT;

    memcpy(jsn2_module_cfg, &buf[index], sizeof(jsn2_module_cfg_t) * JSN2_MODULE_COUNT);
    index += sizeof(jsn2_module_cfg_t) * JSN2_MODULE_COUNT;

    memcpy(meter_cfg, &buf[index], sizeof(meter_cfg_t) * METER_COUNT);
    index += sizeof(meter_cfg_t) * METER_COUNT;

    memcpy(&general_weather_measure_cfg, &buf[index], sizeof(general_weather_measure_cfg_t));
    index += sizeof(general_weather_measure_cfg_t);

    memcpy(weather_autom_cfg, &buf[index], sizeof(weather_autom_cfg_t) * WEATHER_AUTOM_COUNT);
    index += sizeof(weather_autom_cfg_t) * WEATHER_AUTOM_COUNT;

    memcpy(&temperatures_cfg, &buf[index], sizeof(temperatures_cfg_t));
    index += sizeof(temperatures_cfg_t);

    memcpy(circuit_cfg, &buf[index], sizeof(circuit_cfg_t) * CIRCUIT_COUNT);
    index += sizeof(circuit_cfg_t) * CIRCUIT_COUNT;

    memcpy(&group_cfg, &buf[index], sizeof(group_cfg_t));
    index += sizeof(group_cfg_t);

    memcpy(&io_cfg, &buf[index], sizeof(io_cfg_t));
    index += sizeof(io_cfg_t);

    memcpy(&can_cfg, &buf[index], sizeof(can_cfg_t));
    index += sizeof(can_cfg_t);

    memcpy(&modbus_slave_cfg, &buf[index], sizeof(modbus_slave_cfg_t));
    index += sizeof(modbus_slave_cfg_t);

    memcpy(&tgfm_cfg, &buf[index], sizeof(tgfm_cfg_t));
    index += sizeof(tgfm_cfg_t);

    memcpy(rs_cfg, &buf[index], sizeof(rs_cfg_t) * RS_COUNT);
    index += sizeof(rs_cfg_t) * RS_COUNT;

    memcpy(&eth_cfg, &buf[index], sizeof(eth_cfg_t));
    index += sizeof(eth_cfg_t);
}

bool MainWindow::updateGeneralCfg(void)
{
    QTextCodec *codec = QTextCodec::codecForName("ISO 8859-2");

    memset(&general_cfg, 0, sizeof(general_cfg_t));
    strcpy(general_cfg.name, codec->fromUnicode(ui->objName->text()).data());
    strncpy(general_cfg.description, codec->fromUnicode(ui->objDescription->toPlainText()).data(), 255);
    general_cfg.cir_count = ui->cirCount->value();
    general_cfg.weather_autom_count = ui->weatherAutomCount->value();
    general_cfg.ctrl_group_cnt = 6;
    general_cfg.ind_on_time = ui->indOnTime->value();
    general_cfg.phase_asymmetry_check = ui->phaseAssymetryCheck->isChecked();
    general_cfg.phase_assymetry_tolerance = ui->phaseAssymetryTolerance->value();
    general_cfg.min_phase_voltage = ui->minPhaseVoltage->value();
    general_cfg.ctrl_user_level = ui->ctrlUserLevel->value();
    general_cfg.param_edit_user_level = ui->paramEditUserLevel->value();
    general_cfg.screen_saver_enable = ui->screenSaverEnable->isChecked();
    general_cfg.screen_saver_time = ui->screenSaverTime->value();

    return true;
}

bool MainWindow::setGeneralCfg(void)
{
    QTextCodec *codec = QTextCodec::codecForName("ISO 8859-2");

    ui->objName->setText(codec->toUnicode(general_cfg.name));
    ui->objDescription->setPlainText(codec->toUnicode(general_cfg.description));
    ui->cirCount->setValue(general_cfg.cir_count);
    ui->weatherAutomCount->setValue(general_cfg.weather_autom_count);
    ui->indOnTime->setValue(general_cfg.ind_on_time);
    if(general_cfg.phase_asymmetry_check == 0)
        ui->phaseAssymetryCheck->setChecked(false);
    else
        ui->phaseAssymetryCheck->setChecked(true);
    ui->phaseAssymetryTolerance->setValue(general_cfg.phase_assymetry_tolerance);
    ui->minPhaseVoltage->setValue(general_cfg.min_phase_voltage);
    ui->ctrlUserLevel->setValue(general_cfg.ctrl_user_level);
    ui->paramEditUserLevel->setValue(general_cfg.param_edit_user_level);
    if(general_cfg.screen_saver_enable == 0)
        ui->screenSaverEnable->setChecked(false);
    else
        ui->screenSaverEnable->setChecked(true);
    ui->screenSaverTime->setValue(general_cfg.screen_saver_time);

    return true;
}

bool MainWindow::updateIOModuleCfg(void)
{
    int count = ui->ioModulesTable->rowCount();
    int i;
    QComboBox* cmb;
    bool ok = true;

    memset(io_module_cfg, 0, sizeof(io_module_cfg_t) * IO_MODULE_COUNT);
    for(i = 0; i < count; i++)
    {
        cmb = (QComboBox*)ui->ioModulesTable->cellWidget(i, 0);
        io_module_cfg[i].type = cmb->currentData().toInt();
        io_module_cfg[i].addr = ui->ioModulesTable->item(i, 1)->text().toInt(&ok, 0);
        if(ok == false)
            break;
    }

    return ok;
}

bool MainWindow::setIOModuleCfg(void)
{
    QTableWidget *ioModuleTable = ui->ioModulesTable;
    int i, row;
    QComboBox *cb;

    while(ioModuleTable->rowCount() > 0)
        ioModuleTable->removeRow(ioModuleTable->rowCount() - 1
                                 );
    for(i = 0; i < IO_MODULE_COUNT; i++)
    {
        if(io_module_cfg[i].type != 0)
        {
            cb = new QComboBox();
            cb->addItem(QString("IO10/5"), 1);
            cb->addItem(QString("TH"), 3);
            cb->addItem(QString("I12"), 4);
            cb->addItem(QString("I20"), 5);
            cb->addItem(QString("O10"), 6);
            cb->addItem(QString("CVM"), 13);
            cb->addItem(QString("ISC3"), 10);
            cb->addItem(QString("IO4/7"), 11);
            cb->addItem(QString("O8"), 12);
            cb->addItem(QString("GMR IO"), 14);

            row = ioModuleTable->rowCount();
            ioModuleTable->insertRow(row);
            ioModuleTable->setCellWidget(row, 0, cb);
            cb->setCurrentIndex(cb->findData(io_module_cfg[i].type));
            ioModuleTable->setItem(row, 1, new QTableWidgetItem());
            ioModuleTable->item(row, 1)->setText(QString("0x%1").arg(io_module_cfg[i].addr, 0, 16));
        }
    }
    return true;
}

bool MainWindow::updateJsn2Cfg(void)
{
    int count = ui->jsn2ModulesTable->rowCount();
    int i;
    bool ok = true;

    memset(jsn2_module_cfg, 0, sizeof(jsn2_module_cfg_t) * JSN2_MODULE_COUNT);
    for(i = 0; i < count; i++)
    {
        jsn2_module_cfg[i].type = 1;
        jsn2_module_cfg[i].addr = ui->jsn2ModulesTable->item(i, 0)->text().toInt(&ok, 0);
        if(ok == false)
            break;
    }

    return ok;
}

bool MainWindow::setJsn2Cfg(void)
{
    QTableWidget *jsn2Table = ui->jsn2ModulesTable;
    int i, row;

    jsn2Table->clear();
    for(i = 0; i < JSN2_MODULE_COUNT; i++)
    {
        if(jsn2_module_cfg[i].type != 0)
        {
            row = jsn2Table->rowCount();
            jsn2Table->insertRow(row);
            jsn2Table->setItem(row, 0, new QTableWidgetItem());
            jsn2Table->item(row, 0)->setText(QString::number(jsn2_module_cfg[i].addr));
        }
    }
    return true;
}

bool MainWindow::updateMeterCfg(void)
{
    int count = ui->energyMetersTab->rowCount();
    int i;

    memset(meter_cfg, 0, sizeof(meter_cfg_t) * METER_COUNT);
    for(i = 0; i < count; i++)
    {
        meter_cfg[i].type = 1;
        strncpy(meter_cfg[i].id, ui->energyMetersTab->item(i, 0)->text().toUtf8().data(), 14);
    }

    return true;
}

bool MainWindow::setMeterCfg(void)
{
    QTableWidget *energyMetersTab = ui->energyMetersTab;
    int i, row;

    energyMetersTab->clear();
    for(i = 0; i < METER_COUNT; i++)
    {
        if(meter_cfg[i].type != 0)
        {
            row = energyMetersTab->rowCount();
            energyMetersTab->insertRow(row);
            energyMetersTab->setItem(row, 0, new QTableWidgetItem());
            energyMetersTab->item(row, 0)->setText(meter_cfg[i].id);
        }
    }
    return true;
}

bool MainWindow::updateGeneralWeatherMeasure(void)
{
    bool ret = true;
    uint8_t addr = ui->generalTempSensorAddr->value();
    uint16_t reg_no = ui->generalTempSensorRegNo->value();
    uint8_t bit_no;

    memset(&general_weather_measure_cfg, 0, sizeof(general_weather_measure_cfg_t));
    general_weather_measure_cfg.temperature_sensor.type = ui->generalTempSensorTypeCmb->currentIndex();
    general_weather_measure_cfg.temperature_sensor.addr = addr;
    general_weather_measure_cfg.temperature_sensor.reg_no = reg_no;

    switch(general_weather_measure_cfg.temperature_sensor.type)
    {
    case 0:     //TH
        if(checkIoMod(addr, 2, "Ogólne pomiary pogody - czujnik temperatury otoczenia") == 1)
            return false;
        break;
    case 1:     //CAN
        break;
    case 2:     //Modbus
        break;
    }

    general_weather_measure_cfg.snow_fall_sensor.type = ui->generalFallSensorTypeCmb->currentIndex();
    addr = ui->generalFallSensorAddr->value();
    reg_no = ui->generalFallSensorRegNo->value();
    bit_no = ui->generalFallSensorBitNo->value();

    switch(general_weather_measure_cfg.snow_fall_sensor.type)
    {
    case 0:     //io
        if(checkIoMod(addr, 0, "Ogólne pomiary pogody - czujnik opadu") == 1)
            return false;
        break;
    case 1:     //can
        break;
    case 2:     //modbus
        break;
    }
    general_weather_measure_cfg.snow_fall_sensor.addr = addr;
    general_weather_measure_cfg.snow_fall_sensor.reg_no = reg_no;
    general_weather_measure_cfg.snow_fall_sensor.bit_no = bit_no;

    return ret;
}

bool MainWindow::setGeneralWeatherMeasure(void)
{
    ui->generalTempSensorTypeCmb->setCurrentIndex(general_weather_measure_cfg.temperature_sensor.type);
    ui->generalTempSensorAddr->setValue(general_weather_measure_cfg.temperature_sensor.addr);
    ui->generalTempSensorRegNo->setValue(general_weather_measure_cfg.temperature_sensor.reg_no);

    ui->generalFallSensorTypeCmb->setCurrentIndex(general_weather_measure_cfg.snow_fall_sensor.type);
    ui->generalFallSensorAddr->setValue(general_weather_measure_cfg.snow_fall_sensor.addr);
    ui->generalFallSensorBitNo->setValue(general_weather_measure_cfg.snow_fall_sensor.bit_no);
    ui->generalFallSensorRegNo->setValue(general_weather_measure_cfg.snow_fall_sensor.reg_no);

    return true;
}

bool MainWindow::setWeatherAutomCfg(int id)
{
    ui->sensorColdTypeCmb->setCurrentIndex(weather_autom_cfg[id].t_cold.type);
    ui->sensorColdAddr->setValue(weather_autom_cfg[id].t_cold.addr);
    ui->sensorColdRegNo->setValue(weather_autom_cfg[id].t_cold.reg_no);

    ui->sensorHotTypeCmb->setCurrentIndex(weather_autom_cfg[id].t_hot.type);
    ui->sensorHotAddr->setValue(weather_autom_cfg[id].t_hot.addr);
    ui->sensorHotRegNo->setValue(weather_autom_cfg[id].t_hot.reg_no);

    ui->blowSensTypeCmb->setCurrentIndex(weather_autom_cfg[id].snow_blow_sensor.type);
    ui->blowSensorAddr->setValue(weather_autom_cfg[id].snow_blow_sensor.addr);
    ui->blowSensorRegNo->setValue(weather_autom_cfg[id].snow_blow_sensor.reg_no);
    ui->blowSensorBitNo->setValue(weather_autom_cfg[id].snow_blow_sensor.bit_no);

    if(weather_autom_cfg[id].sensor_pwr_ctrl.active == 0)
        ui->sensorPwrCtrlChk->setChecked(false);
    else
        ui->sensorPwrCtrlChk->setChecked(true);
    ui->sensorPwrCtrlIOMod->setValue(weather_autom_cfg[id].sensor_pwr_ctrl.module_id);
    ui->sensorPwrCtrlBitNo->setValue(weather_autom_cfg[id].sensor_pwr_ctrl.bit_no);

    ui->referenceCirNo->setValue(weather_autom_cfg[id].referenceCircuitNo);

    return true;
}

bool MainWindow::setCircuitCfg(int id)
{
    QTextCodec *codec = QTextCodec::codecForName("ISO 8859-2");

    ui->cirNameEdit->setText(codec->toUnicode(circuit_cfg[id].name));
    if(circuit_cfg[id].active == 0)
        ui->cirActiveChk->setChecked(false);
    else
        ui->cirActiveChk->setChecked(true);
    if(circuit_cfg[id].reference == 0)
        ui->cirReferenceChk->setChecked(false);
    else
        ui->cirReferenceChk->setChecked(true);
    ui->cirTypeCmb->setCurrentIndex(circuit_cfg[id].type);
    ui->cirGroupNo->setValue(circuit_cfg[id].group_id);
    ui->cirWeatherAutomNo->setValue(circuit_cfg[id].weather_autom_id);

    if(circuit_cfg[id].phase_cfg[0].active == 0)
        ui->l1ActiveChk->setChecked(false);
    else
        ui->l1ActiveChk->setChecked(true);
    if(circuit_cfg[id].phase_cfg[0].conf_input.active ==0)
        ui->l1ConfActiveChk->setChecked(false);
    else
        ui->l1ConfActiveChk->setChecked(true);
    ui->l1ConfIOMod->setValue(circuit_cfg[id].phase_cfg[0].conf_input.module_id);
    ui->l1ConfBitNo->setValue(circuit_cfg[id].phase_cfg[0].conf_input.bit_no);

    ui->l1CvmId->setValue(circuit_cfg[id].phase_cfg[0].cvm_id);
    ui->l1CvmCh->setValue(circuit_cfg[id].phase_cfg[0].cvm_ch_id);

    ui->p1Nominal->setValue(circuit_cfg[id].phase_cfg[0].p_nom / 10.0);
    ui->p1Tol->setValue(circuit_cfg[id].phase_cfg[0].p_tol / 10.0);

    if(circuit_cfg[id].phase_cfg[1].active == 0)
        ui->l2ActiveChk->setChecked(false);
    else
        ui->l2ActiveChk->setChecked(true);
    if(circuit_cfg[id].phase_cfg[1].conf_input.active ==0)
        ui->l2ConfActiveChk->setChecked(false);
    else
        ui->l2ConfActiveChk->setChecked(true);
    ui->l2ConfIOMod->setValue(circuit_cfg[id].phase_cfg[1].conf_input.module_id);
    ui->l2ConfBitNo->setValue(circuit_cfg[id].phase_cfg[1].conf_input.bit_no);

    ui->l2CvmId->setValue(circuit_cfg[id].phase_cfg[1].cvm_id);
    ui->l2CvmCh->setValue(circuit_cfg[id].phase_cfg[1].cvm_ch_id);

    ui->p2Nominal->setValue(circuit_cfg[id].phase_cfg[1].p_nom / 10.0);
    ui->p2Tol->setValue(circuit_cfg[id].phase_cfg[1].p_tol / 10.0);

    if(circuit_cfg[id].phase_cfg[2].active == 0)
        ui->l3ActiveChk->setChecked(false);
    else
        ui->l3ActiveChk->setChecked(true);
    if(circuit_cfg[id].phase_cfg[2].conf_input.active ==0)
        ui->l3ConfActiveChk->setChecked(false);
    else
        ui->l3ConfActiveChk->setChecked(true);
    ui->l3ConfIOMod->setValue(circuit_cfg[id].phase_cfg[2].conf_input.module_id);
    ui->l3ConfBitNo->setValue(circuit_cfg[id].phase_cfg[2].conf_input.bit_no);

    ui->l3CvmId->setValue(circuit_cfg[id].phase_cfg[2].cvm_id);
    ui->l3CvmCh->setValue(circuit_cfg[id].phase_cfg[2].cvm_ch_id);

    ui->p3Nominal->setValue(circuit_cfg[id].phase_cfg[2].p_nom / 10.0);
    ui->p3Tol->setValue(circuit_cfg[id].phase_cfg[2].p_tol / 10.0);

    ui->cirRelIOMod->setValue(circuit_cfg[id].relay.module_id);
    ui->cirRelBitNo->setValue(circuit_cfg[id].relay.bit_no);
    ui->cirRelConfIOMod->setValue(circuit_cfg[id].rel_conf.module_id);
    ui->cirRelConfBitNo->setValue(circuit_cfg[id].rel_conf.bit_no);

    return true;
}

bool MainWindow::updateTemperaturesCfg(void)
{
    memset(&temperatures_cfg, 0, sizeof(temperatures_cfg_t));

    QDoubleSpinBox *spin;

    spin = (QDoubleSpinBox*)ui->weatherAutomTempTable->cellWidget(0, 0);
    temperatures_cfg.t_r_on_fr = spin->value() * 10;
    spin = (QDoubleSpinBox*)ui->weatherAutomTempTable->cellWidget(0, 1);
    temperatures_cfg.t_r_off_fr = spin->value() * 10;
    spin = (QDoubleSpinBox*)ui->weatherAutomTempTable->cellWidget(1, 0);
    temperatures_cfg.t_r_on_wet = spin->value() * 10;
    spin = (QDoubleSpinBox*)ui->weatherAutomTempTable->cellWidget(1, 1);
    temperatures_cfg.t_r_off_wet = spin->value() * 10;
    spin = (QDoubleSpinBox*)ui->weatherAutomTempTable->cellWidget(2, 0);
    temperatures_cfg.t_r_on_sn = spin->value() * 10;
    spin = (QDoubleSpinBox*)ui->weatherAutomTempTable->cellWidget(2, 1);
    temperatures_cfg.t_r_off_sn = spin->value() * 10;

    temperatures_cfg.t_frost_on_r = ui->frostTempIn->value() * 10;
    temperatures_cfg.t_frost_off_r = ui->frostTempOut->value() * 10;

    temperatures_cfg.t_frost_on_l = ui->tLocksOn->value() * 10;
    temperatures_cfg.t_frost_off_l = ui->tLocksOff->value() * 10;
    return true;
}

bool MainWindow::setTemperaturesCfg(void)
{
    QDoubleSpinBox *spin;

    spin = (QDoubleSpinBox*)ui->weatherAutomTempTable->cellWidget(0, 0);
    spin->setValue(temperatures_cfg.t_r_on_fr / 10.0);
    spin = (QDoubleSpinBox*)ui->weatherAutomTempTable->cellWidget(0, 1);
    spin->setValue(temperatures_cfg.t_r_off_fr / 10.0);
    spin = (QDoubleSpinBox*)ui->weatherAutomTempTable->cellWidget(1, 0);
    spin->setValue(temperatures_cfg.t_r_on_wet / 10.0);
    spin = (QDoubleSpinBox*)ui->weatherAutomTempTable->cellWidget(1, 1);
    spin->setValue(temperatures_cfg.t_r_off_wet / 10.0);
    spin = (QDoubleSpinBox*)ui->weatherAutomTempTable->cellWidget(2, 0);
    spin->setValue(temperatures_cfg.t_r_on_sn / 10.0);
    spin = (QDoubleSpinBox*)ui->weatherAutomTempTable->cellWidget(2, 1);
    spin->setValue(temperatures_cfg.t_r_off_sn / 10.0);

    ui->frostTempIn->setValue(temperatures_cfg.t_frost_on_r / 10.0);
    ui->frostTempOut->setValue(temperatures_cfg.t_frost_off_r / 10.0);

    ui->tLocksOn->setValue(temperatures_cfg.t_frost_on_l / 10.0);
    ui->tLocksOff->setValue(temperatures_cfg.t_frost_off_l / 10.0);
    return true;
}

bool MainWindow::updateIoCfg(void)
{
    memset(&io_cfg, 0, sizeof(io_cfg_t));

    io_cfg.dor.active = ui->dorChk->isChecked();
    io_cfg.dor.module_id = ui->dorIOMod->value();
    io_cfg.dor.bit_no = ui->dorBitNo->value();
    if(io_cfg.dor.active != 0)
        checkIoMod(io_cfg.dor.module_id, 0, "Otwarcie drzwi szafy");

    io_cfg.break_in.active = ui->breakInChk->isChecked();
    io_cfg.break_in.module_id = ui->breakInIOMod->value();
    io_cfg.break_in.bit_no = ui->breakInBitNo->value();
    if(io_cfg.break_in.active != 0)
        checkIoMod(io_cfg.break_in.module_id, 0, "Otwarcie klapy transformatora");

    io_cfg.surge_protect.active = ui->surgeProtectChk->isChecked();
    io_cfg.surge_protect.module_id = ui->surgeProtectIOMod->value();
    io_cfg.surge_protect.bit_no = ui->surgeProtectBitNo->value();
    if(io_cfg.surge_protect.active != 0)
        checkIoMod(io_cfg.surge_protect.module_id, 0, "Potwierdzenie zabezpieczenia antyprzepięciowego");

    io_cfg.remote_ctrl.active = ui->remoteCtrlChk->isChecked();
    io_cfg.remote_ctrl.module_id = ui->remoteCtrlIOMod->value();
    io_cfg.remote_ctrl.bit_no = ui->remoteCtrlBitNo->value();
    if(io_cfg.remote_ctrl.active != 0)
        checkIoMod(io_cfg.remote_ctrl.module_id, 0, "Sterowanie zdalne");

    io_cfg.local_ctrl.active = ui->localCtrlChk->isChecked();
    io_cfg.local_ctrl.module_id = ui->localCtrlIOMod->value();
    io_cfg.local_ctrl.bit_no = ui->localCtrlBitNo->value();
    if(io_cfg.local_ctrl.active != 0)
        checkIoMod(io_cfg.local_ctrl.module_id, 0, "Sterowanie lokalne");

    io_cfg.hand_ctrl.active = ui->handCtrlChk->isChecked();
    io_cfg.hand_ctrl.module_id = ui->handCtrlIOMod->value();
    io_cfg.hand_ctrl.bit_no = ui->handCtrlBitNo->value();
    if(io_cfg.hand_ctrl.active != 0)
        checkIoMod(io_cfg.hand_ctrl.module_id, 0, "Sterowanie ręczne");

    return true;
}

bool MainWindow::setIoCfg(void)
{
    if(io_cfg.dor.active == 0)
        ui->dorChk->setChecked(false);
    else
        ui->dorChk->setChecked(true);
    ui->dorIOMod->setValue(io_cfg.dor.module_id);
    ui->dorBitNo->setValue(io_cfg.dor.bit_no);

    if(io_cfg.break_in.active == 0)
        ui->breakInChk->setChecked(false);
    else
        ui->breakInChk->setChecked(true);
    ui->breakInIOMod->setValue(io_cfg.break_in.module_id);
    ui->breakInBitNo->setValue(io_cfg.break_in.bit_no);

    if(io_cfg.surge_protect.active == 0)
        ui->surgeProtectChk->setChecked(false);
    else
        ui->surgeProtectChk->setChecked(true);
    ui->surgeProtectIOMod->setValue(io_cfg.surge_protect.module_id);
    ui->surgeProtectBitNo->setValue(io_cfg.surge_protect.bit_no);

    if(io_cfg.remote_ctrl.active == 0)
        ui->remoteCtrlChk->setChecked(false);
    else
        ui->remoteCtrlChk->setChecked(true);
    ui->remoteCtrlIOMod->setValue(io_cfg.remote_ctrl.module_id);
    ui->remoteCtrlBitNo->setValue(io_cfg.remote_ctrl.bit_no);

    ui->localCtrlChk->setChecked(true);
    ui->localCtrlIOMod->setValue(io_cfg.local_ctrl.module_id);
    ui->localCtrlBitNo->setValue(io_cfg.local_ctrl.bit_no);

    ui->handCtrlChk->setChecked(true);
    ui->handCtrlIOMod->setValue(io_cfg.hand_ctrl.module_id);
    ui->handCtrlBitNo->setValue(io_cfg.hand_ctrl.bit_no);

    return true;
}

bool MainWindow::updateModbusSlaveCfg(void)
{
    memset(&modbus_slave_cfg, 0, sizeof(modbus_slave_cfg_t));
    modbus_slave_cfg.active = ui->modbusSlaveActiveChk->isChecked();
    modbus_slave_cfg.addr = ui->modbusSlaveAddr->value();
    modbus_slave_cfg.transmission_medium = ui->modbusSlaveMediumCmb->currentIndex();
    modbus_slave_cfg.port_no = ui->modbusSlavePort->value();
    modbus_slave_cfg.accept_cmd = ui->modbusSlaveAcceptCmdChk->isChecked();
    return true;
}

bool MainWindow::setModbusSlaveCfg(void)
{
    if(modbus_slave_cfg.active == 0)
        ui->modbusSlaveActiveChk->setChecked(false);
    else
        ui->modbusSlaveActiveChk->setChecked(true);
    ui->modbusSlaveAddr->setValue(modbus_slave_cfg.addr);
    ui->modbusSlaveMediumCmb->setCurrentIndex(modbus_slave_cfg.transmission_medium);
    ui->modbusSlavePort->setValue(modbus_slave_cfg.port_no);
    if(modbus_slave_cfg.accept_cmd == 0)
        ui->modbusSlaveAcceptCmdChk->setChecked(false);
    else
        ui->modbusSlaveAcceptCmdChk->setChecked(true);

    return true;
}

static bool strToIp(uint8_t *ip, QString string)
{
    QStringList list;
    int i;
    int val;
    bool err = false;
    bool ok;

    list = string.split(".");
    if(list.size() < 4)
    {
        err = true;
    }
    else
    {
        for(i = 0; i < 4; i++)
        {
            val = list[i].toInt(&ok);
            if((ok == false) || (val < 0) || (val > 255))
            {
                err = true;
                break;
            }
            else
            {
                ip[i] = val;
            }
        }
    }
    return err;
}

bool MainWindow::updateEthCfg(void)
{
    bool ip_err = false;
    bool mask_err = false;
    bool gateway_err = false;

    memset(&eth_cfg, 0, sizeof(eth_cfg_t));
    ip_err = strToIp(eth_cfg.ip, ui->ipAddrEdit->text());
    if(ip_err)
        QMessageBox::critical(this, "Błąd", "Błąd w adresie IP");

    mask_err = strToIp(eth_cfg.mask, ui->maskEdit->text());
    if(mask_err)
        QMessageBox::critical(this, "Błąd", "Błąd w masce podsieci");

    gateway_err = strToIp(eth_cfg.gateway, ui->gatewayEdit->text());
    if(gateway_err)
        QMessageBox::critical(this, "Błąd", "Błąd w adresie IP bramy domyślnej");

    return !(ip_err || mask_err || gateway_err);
}

bool MainWindow::setEthCfg(void)
{
    ui->ipAddrEdit->setText(QString("%1.%2.%3.%4").arg(eth_cfg.ip[0]).arg(eth_cfg.ip[1]).arg(eth_cfg.ip[2]).arg(eth_cfg.ip[3]));
    ui->maskEdit->setText(QString("%1.%2.%3.%4").arg(eth_cfg.mask[0]).arg(eth_cfg.mask[1]).arg(eth_cfg.mask[2]).arg(eth_cfg.mask[3]));
    ui->gatewayEdit->setText(QString("%1.%2.%3.%4").arg(eth_cfg.gateway[0]).arg(eth_cfg.gateway[1]).arg(eth_cfg.gateway[2]).arg(eth_cfg.gateway[3]));

    return true;
}

bool MainWindow::updateRsCfg(void)
{
    bool ok;
    bool err = false;

    memset(rs_cfg, 0, sizeof(rs_cfg_t) * RS_COUNT);

    rs_cfg[0].active = ui->rsActiveChk->isChecked();
    rs_cfg[0].type = 0;			//fizyczny
    rs_cfg[0].baud = ui->rsBaudrateCmb->currentText().toInt(&ok);
    rs_cfg[0].stop_bits = ui->rsStopBitsCmb->currentIndex();
    rs_cfg[0].parity = ui->rsParityCmb->currentIndex();
    rs_cfg[0].protocol = ui->rsProtocolCmb->currentIndex();
    if((ok == false) && (rs_cfg[0].active != 0))
        QMessageBox::critical(this, "Błąd", "RS232/485 - błędnie wprowadzone Baudrate");

    rs_cfg[1].active = ui->virtRs1ActiveChk->isChecked();
    rs_cfg[1].type = 1;			//wirtualny
    rs_cfg[1].port = ui->virtRs1Port->value();
    if((err |= strToIp(rs_cfg[1].server_ip, ui->virtRs1IpAddrEdit->text())) && (rs_cfg[1].active != 0))
        QMessageBox::critical(this, "Błąd", "Błąd w adresie IP wirtualnego portu 1");
    rs_cfg[1].protocol = ui->virtRs1ProtocolCmb->currentIndex();

    rs_cfg[2].active = ui->virtRs2ActiveChk->isChecked();
    rs_cfg[2].type = 1;			//wirtualny
    rs_cfg[2].port = ui->virtRs2Port->value();
    if((err |= strToIp(rs_cfg[2].server_ip, ui->virtRs2IpAddrEdit->text())) && (rs_cfg[2].active != 0))
        QMessageBox::critical(this, "Błąd", "Błąd w adresie IP wirtualnego portu 2");
    rs_cfg[2].protocol = ui->virtRs2ProtocolCmb->currentIndex();

    rs_cfg[3].active = ui->virtRs3ActiveChk->isChecked();
    rs_cfg[3].type = 1;			//wirtualny
    rs_cfg[3].port = ui->virtRs3Port->value();
    if((err |= strToIp(rs_cfg[3].server_ip, ui->virtRs3IpAddrEdit->text())) && (rs_cfg[3].active != 0))
        QMessageBox::critical(this, "Błąd", "Błąd w adresie IP wirtualnego portu 3");
    rs_cfg[3].protocol = ui->virtRs3ProtocolCmb->currentIndex();

    rs_cfg[4].active = ui->virtRs4ActiveChk->isChecked();
    rs_cfg[4].type = 1;			//wirtualny
    rs_cfg[4].port = ui->virtRs4Port->value();
    if((err |= strToIp(rs_cfg[4].server_ip, ui->virtRs4IpAddrEdit->text())) && (rs_cfg[4].active != 0))
        QMessageBox::critical(this, "Błąd", "Błąd w adresie IP wirtualnego portu 4");
    rs_cfg[4].protocol = ui->virtRs4ProtocolCmb->currentIndex();

    return (err | ok);
}

bool MainWindow::setRsCfg(void)
{
    if(rs_cfg[0].active == 0)
        ui->rsActiveChk->setChecked(false);
    else
        ui->rsActiveChk->setChecked(true);
    ui->rsBaudrateCmb->setCurrentText(QString::number(rs_cfg[0].baud));
    ui->rsStopBitsCmb->setCurrentIndex(rs_cfg[0].stop_bits);
    ui->rsParityCmb->setCurrentIndex(rs_cfg[0].parity);
    ui->rsProtocolCmb->setCurrentIndex(rs_cfg[0].protocol);

    if(rs_cfg[1].active == 0)
        ui->virtRs1ActiveChk->setChecked(false);
    else
        ui->virtRs1ActiveChk->setChecked(true);
    ui->virtRs1Port->setValue(rs_cfg[1].port);
    ui->virtRs1IpAddrEdit->setText(QString("%1.%2.%3.%4").arg(rs_cfg[1].server_ip[0]).arg(rs_cfg[1].server_ip[1]).arg(rs_cfg[1].server_ip[2]).arg(rs_cfg[1].server_ip[3]));
    ui->virtRs1ProtocolCmb->setCurrentIndex(rs_cfg[1].protocol);

    if(rs_cfg[2].active == 0)
        ui->virtRs2ActiveChk->setChecked(false);
    else
        ui->virtRs2ActiveChk->setChecked(true);
    ui->virtRs2Port->setValue(rs_cfg[2].port);
    ui->virtRs2IpAddrEdit->setText(QString("%1.%2.%3.%4").arg(rs_cfg[2].server_ip[0]).arg(rs_cfg[2].server_ip[1]).arg(rs_cfg[2].server_ip[2]).arg(rs_cfg[2].server_ip[3]));
    ui->virtRs2ProtocolCmb->setCurrentIndex(rs_cfg[2].protocol);

    if(rs_cfg[3].active == 0)
        ui->virtRs3ActiveChk->setChecked(false);
    else
        ui->virtRs3ActiveChk->setChecked(true);
    ui->virtRs3Port->setValue(rs_cfg[3].port);
    ui->virtRs3IpAddrEdit->setText(QString("%1.%2.%3.%4").arg(rs_cfg[3].server_ip[0]).arg(rs_cfg[3].server_ip[1]).arg(rs_cfg[3].server_ip[2]).arg(rs_cfg[3].server_ip[3]));
    ui->virtRs1ProtocolCmb->setCurrentIndex(rs_cfg[3].protocol);

    if(rs_cfg[4].active == 0)
        ui->virtRs4ActiveChk->setChecked(false);
    else
        ui->virtRs4ActiveChk->setChecked(true);
    ui->virtRs4Port->setValue(rs_cfg[4].port);
    ui->virtRs4IpAddrEdit->setText(QString("%1.%2.%3.%4").arg(rs_cfg[4].server_ip[0]).arg(rs_cfg[4].server_ip[1]).arg(rs_cfg[4].server_ip[2]).arg(rs_cfg[4].server_ip[3]));
    ui->virtRs4ProtocolCmb->setCurrentIndex(rs_cfg[4].protocol);

    return true;
}

int MainWindow::checkIoMod(uint8_t module_id, uint8_t input_output_type, QString text)
{
    //type: 0 - digital input
    //      1 - digital output
    //      2 - TH
    //      3 - CVM
    QComboBox *cb;

    if(module_id > ui->ioModulesTable->rowCount())
        return QMessageBox::critical(this, "Błąd", text + QString("\nNie ma modułu IO o podanym numerze."), "Ignoruj", "Popraw");

    switch(input_output_type)
    {
    case 0:     //digital input
        cb = (QComboBox*)ui->ioModulesTable->cellWidget(module_id - 1, 0);
        if((cb->currentIndex() != 0) &&         /* io10/5 */
                (cb->currentIndex() != 1) &&    /* th     */
                (cb->currentIndex() != 2) &&    /* i12    */
                (cb->currentIndex() != 3) &&    /* i20    */
                (cb->currentIndex() != 7) &&    /* io4/7  */
                (cb->currentIndex() != 9))      /* gmr io */
        {
            return QMessageBox::critical(this, "Błąd", text + QString("\nModuł o podanym numerze nie ma wejść cyfrowych."), "Ignoruj", "Popraw");
        }
        break;
    case 1:     //digital output
        cb = (QComboBox*)ui->ioModulesTable->cellWidget(module_id - 1, 0);
        if((cb->currentIndex() != 0) &&         /* io10/5 */
                (cb->currentIndex() != 4) &&    /* o10    */
                (cb->currentIndex() != 7) &&    /* io4/7  */
                (cb->currentIndex() != 9))      /* gmr io */
        {
            return QMessageBox::critical(this, "Błąd", text +  QString("\nModuł o podanym numerze nie ma wyjść cyfrowych."), "Ignoruj", "Popraw");
        }
        break;
    case 2:     //TH
        cb = (QComboBox*)ui->ioModulesTable->cellWidget(module_id - 1, 0);
        if(cb->currentIndex() != 1)
        {
            return QMessageBox::critical(this, "Błąd", text + QString("\nModuł o podanym numerze nie jest modułem TH."), "Ignoruj", "Popraw");
        }
        break;
    case 3:     //CVM
        cb = (QComboBox*)ui->ioModulesTable->cellWidget(module_id - 1, 0);
        if(cb->currentIndex() != 5)         /* CVM */
        {
            return QMessageBox::critical(this, "Błąd", text + QString("\nModuł o podanym numerze nie jest modułem CVM."), "Ignoruj", "Popraw");
        }
        break;
    }

    return 0;
}

void MainWindow::on_addIoModuleBtn_clicked()
{
    QComboBox* cb;
    QComboBox* prev_cb;
    QTableWidgetItem* item;

    int rows = ui->ioModulesTable->rowCount();
    if(rows < IO_MODULE_COUNT)
    {
        ui->ioModulesTable->insertRow(rows);

        cb = new QComboBox();
        cb->addItem(QString("IO10/5"), 1);
        cb->addItem(QString("TH"), 3);
        cb->addItem(QString("I12"), 4);
        cb->addItem(QString("I20"), 5);
        cb->addItem(QString("O10"), 6);
        cb->addItem(QString("CVM"), 13);
        cb->addItem(QString("ISC3"), 10);
        cb->addItem(QString("IO4/7"), 11);
        cb->addItem(QString("O8"), 12);
        cb->addItem(QString("GMR IO"), 14);

        ui->ioModulesTable->setCellWidget(rows, 0, cb);
        ui->ioModulesTable->setItem(rows, 1, new QTableWidgetItem());
        if(rows == 0)
        {
            item = ui->ioModulesTable->item(0, 1);
            item->setText("0x12");
        }
        else
        {
            prev_cb = (QComboBox*)ui->ioModulesTable->cellWidget(rows - 1, 0);
            cb->setCurrentIndex(prev_cb->currentIndex());
            ui->ioModulesTable->item(rows, 1)->setText(QString("0x") + QString::number(ui->ioModulesTable->item(rows - 1, 1)->text().toInt(NULL, 0) + 1, 16));
        }
    }
}

void MainWindow::on_delIoModuleBtn_clicked()
{
    int row;

    if(ui->ioModulesTable->rowCount() > 0)
    {
        row = ui->ioModulesTable->currentRow();
        ui->ioModulesTable->removeRow(row);
    }
}

void MainWindow::on_addJsn2Btn_clicked()
{
    QTableWidgetItem *prev_item;
    int rows = ui->jsn2ModulesTable->rowCount();
    if(rows < JSN2_MODULE_COUNT)
    {
        ui->jsn2ModulesTable->insertRow(rows);
        if(rows > 0)
        {
            prev_item = ui->jsn2ModulesTable->item(rows - 1, 0);
            ui->jsn2ModulesTable->setItem(rows, 0, new QTableWidgetItem());
            ui->jsn2ModulesTable->item(rows, 0)->setText(QString::number(prev_item->text().toInt(NULL, 0) + 1));
        }
    }
}

void MainWindow::on_delJsn2Btn_clicked()
{
    int row;

    if(ui->jsn2ModulesTable->rowCount() > 0)
    {
        row = ui->jsn2ModulesTable->currentRow();
        ui->jsn2ModulesTable->removeRow(row);
    }
}

void MainWindow::on_addMeterBtn_clicked()
{
    int rows = ui->energyMetersTab->rowCount();
    if(rows < METER_COUNT)
    {
        ui->energyMetersTab->insertRow(rows);
    }
}

void MainWindow::on_delMeterBtn_clicked()
{
    int row;

    if(ui->energyMetersTab->rowCount() > 0)
    {
        row = ui->energyMetersTab->currentRow();
        ui->energyMetersTab->removeRow(row);
    }
}

void MainWindow::on_generalTempSensorTypeCmb_currentIndexChanged(int index)
{
    switch(index)
    {
    case 0:     //TH
        ui->generalTempSensorAddrLbl->setText("Nr modułu TH");
        ui->generalTempSensorAddr->setMinimum(1);
        ui->generalTempSensorAddr->setMaximum(IO_MODULE_COUNT);
        ui->generalTempSensorRegNoLbl->setText("Nr kanału TH");
        ui->generalTempSensorRegNo->setMinimum(1);
        ui->generalTempSensorRegNo->setMaximum(4);
        break;
    case 1:     //CAN
        ui->generalTempSensorAddrLbl->setText("Adres CAN");
        ui->generalTempSensorAddr->setMinimum(1);
        ui->generalTempSensorAddr->setMaximum(255);
        ui->generalTempSensorRegNoLbl->setText("Nr meldunku");
        ui->generalTempSensorRegNo->setMinimum(0);
        ui->generalTempSensorRegNo->setMaximum(127);
        break;
    case 2:     //Modbus
        ui->generalTempSensorAddrLbl->setText("Adres Modbus");
        ui->generalTempSensorAddr->setMinimum(1);
        ui->generalTempSensorAddr->setMaximum(255);
        ui->generalTempSensorRegNoLbl->setText("Nr rejestru");
        ui->generalTempSensorRegNo->setMinimum(0);
        ui->generalTempSensorRegNo->setMaximum(65535);
        break;
    }
}

void MainWindow::on_generalFallSensorTypeCmb_currentIndexChanged(int index)
{
    switch(index)
    {
    case 0:     //IO
        ui->generalFallSensorAddrLbl->setText("Nr modułu IO");
        ui->generalFallSensorAddr->setMinimum(1);
        ui->generalFallSensorAddr->setMaximum(IO_MODULE_COUNT);
        ui->generalFallSensorBitNoLbl->setText("Nr wejścia");
        ui->generalFallSensorBitNo->setMinimum(1);
        ui->generalFallSensorBitNo->setMaximum(20);
        ui->generalFallSensorRegNoLbl->setVisible(false);
        ui->generalFallSensorRegNo->setVisible(false);
        break;
    case 1:     //CAN
        ui->generalFallSensorAddrLbl->setText("Adres CAN");
        ui->generalFallSensorAddr->setMinimum(1);
        ui->generalFallSensorAddr->setMaximum(255);
        ui->generalFallSensorBitNoLbl->setText("Nr bitu");
        ui->generalFallSensorBitNo->setMinimum(1);
        ui->generalFallSensorBitNo->setMaximum(16);
        ui->generalFallSensorRegNoLbl->setVisible(true);
        ui->generalFallSensorRegNoLbl->setText("Nr meldunku");
        ui->generalFallSensorRegNo->setMinimum(0);
        ui->generalFallSensorRegNo->setMaximum(127);
        ui->generalFallSensorRegNo->setVisible(true);
        break;
    case 2:     //Modbus
        ui->generalFallSensorAddrLbl->setText("Adres Modbus");
        ui->generalFallSensorAddr->setMinimum(1);
        ui->generalFallSensorAddr->setMaximum(255);
        ui->generalFallSensorBitNoLbl->setText("Nr bitu");
        ui->generalFallSensorBitNo->setMinimum(1);
        ui->generalFallSensorBitNo->setMaximum(16);
        ui->generalFallSensorRegNoLbl->setVisible(true);
        ui->generalFallSensorRegNoLbl->setText("Nr rejestru");
        ui->generalFallSensorRegNo->setMinimum(0);
        ui->generalFallSensorRegNo->setMaximum(65535);
        ui->generalFallSensorRegNo->setVisible(true);
        break;
    }
}

void MainWindow::on_weatherAutomList_currentRowChanged(int currentRow)
{
    setWeatherAutomCfg(currentRow);
}

void MainWindow::on_editWeatherAutomBtn_clicked()
{
    int currentRow = ui->weatherAutomList->currentRow();

    if(ui->editWeatherAutomBtn->text() == "Zmień ustawienia")
    {
        ui->editWeatherAutomBtn->setText("Zapisz zmiany");
        ui->weatherAutomList->setEnabled(false);
        ui->sensorColdFrame->setEnabled(true);
        ui->sensorHotFrame->setEnabled(true);
        ui->blowSensorFRame->setEnabled(true);
        ui->sensorPwrCtrlFrame->setEnabled(true);
        ui->referenceCirNo->setEnabled(true);
    }
    else
    {
        memset(&weather_autom_cfg[currentRow], 0, sizeof(weather_autom_cfg_t));
        weather_autom_cfg[currentRow].t_cold.type = ui->sensorColdTypeCmb->currentIndex();
        weather_autom_cfg[currentRow].t_cold.addr = ui->sensorColdAddr->value();
        weather_autom_cfg[currentRow].t_cold.reg_no = ui->sensorColdRegNo->value();

        weather_autom_cfg[currentRow].t_hot.type = ui->sensorHotTypeCmb->currentIndex();
        weather_autom_cfg[currentRow].t_hot.addr = ui->sensorHotAddr->value();
        weather_autom_cfg[currentRow].t_hot.reg_no = ui->sensorHotRegNo->value();

        weather_autom_cfg[currentRow].snow_blow_sensor.type = ui->blowSensTypeCmb->currentIndex();
        weather_autom_cfg[currentRow].snow_blow_sensor.addr = ui->blowSensorAddr->value();
        weather_autom_cfg[currentRow].snow_blow_sensor.reg_no = ui->blowSensorRegNo->value();
        weather_autom_cfg[currentRow].snow_blow_sensor.bit_no = ui->blowSensorBitNo->value();

        weather_autom_cfg[currentRow].sensor_pwr_ctrl.active = ui->sensorPwrCtrlChk->isChecked();
        weather_autom_cfg[currentRow].sensor_pwr_ctrl.module_id = ui->sensorPwrCtrlIOMod->value();
        weather_autom_cfg[currentRow].sensor_pwr_ctrl.bit_no = ui->sensorPwrCtrlBitNo->value();

        weather_autom_cfg[currentRow].referenceCircuitNo = ui->referenceCirNo->value();

        switch(weather_autom_cfg[currentRow].snow_blow_sensor.type)
        {
        case 0:     //io
            if(checkIoMod(weather_autom_cfg[currentRow].snow_blow_sensor.addr, 0, "Czujnik śniegu nawianego") == 1)
                return;
            break;
        case 1:     //can
            break;
        case 2:     //modbus
            break;
        }

        switch(weather_autom_cfg[currentRow].t_hot.type)
        {
        case 0:     //TH
            if(checkIoMod(weather_autom_cfg[currentRow].t_hot.addr, 2, "Czujnik temperatury szyny grzanej") == 1)
                return;
            break;
        case 1:     //CAN
            break;
        case 2:     //Modbus
            break;
        }

        switch(weather_autom_cfg[currentRow].t_cold.type)
        {
        case 0:     //TH
            if(checkIoMod(weather_autom_cfg[currentRow].t_cold.addr, 2, "Czujnik temperatury szyny zimnej") == 1)
                return;
            break;
        case 1:     //CAN
            break;
        case 2:     //Modbus
            break;
        }

        if(weather_autom_cfg[currentRow].sensor_pwr_ctrl.active != 0)
        {
            if(checkIoMod(weather_autom_cfg[currentRow].sensor_pwr_ctrl.module_id, 2, "Potwierdzenie zasilania czujników") == 1)
                return;
        }

        ui->editWeatherAutomBtn->setText("Zmień ustawienia");
        ui->weatherAutomList->setEnabled(true);
        ui->sensorColdFrame->setEnabled(false);
        ui->sensorHotFrame->setEnabled(false);
        ui->blowSensorFRame->setEnabled(false);
        ui->sensorPwrCtrlFrame->setEnabled(false);
        ui->referenceCirNo->setEnabled(false);
    }
}

void MainWindow::on_weatherAutomCount_valueChanged(int arg1)
{
    int rows = ui->weatherAutomList->count();
    int i;
    QListWidgetItem *itm;

    if((arg1 > rows) && (arg1 <= WEATHER_AUTOM_COUNT))
    {
        for(i = rows; i < arg1; i++)
            ui->weatherAutomList->addItem(QString("Automat pogodowy %1").arg(i + 1));
    }
    else if(arg1 < rows)
    {
        for(i = (rows - 1); i >= arg1; i--)
        {
            itm = ui->weatherAutomList->takeItem(i);
            delete itm;
        }
    }
}

void MainWindow::on_sensorColdTypeCmb_currentIndexChanged(int index)
{
    switch(index)
    {
    case 0:     //TH
        ui->sensorColdAddrLbl->setText("Nr modułu TH");
        ui->sensorColdAddr->setMinimum(1);
        ui->sensorColdAddr->setMaximum(IO_MODULE_COUNT);
        ui->sensorColdRegNoLbl->setText("Nr kanału TH");
        ui->sensorColdRegNo->setMinimum(1);
        ui->sensorColdRegNo->setMaximum(4);
        break;
    case 1:     //CAN
        ui->sensorColdAddrLbl->setText("Adres CAN");
        ui->sensorColdAddr->setMinimum(1);
        ui->sensorColdAddr->setMaximum(255);
        ui->sensorColdRegNoLbl->setText("Nr meldunku");
        ui->sensorColdRegNo->setMinimum(0);
        ui->sensorColdRegNo->setMaximum(127);
        break;
    case 2:     //Modbus
        ui->sensorColdAddrLbl->setText("Adres Modbus");
        ui->sensorColdAddr->setMinimum(1);
        ui->sensorColdAddr->setMaximum(255);
        ui->sensorColdRegNoLbl->setText("Nr rejestru");
        ui->sensorColdRegNo->setMinimum(0);
        ui->sensorColdRegNo->setMaximum(65535);
        break;
    }
}

void MainWindow::on_sensorHotTypeCmb_currentIndexChanged(int index)
{
    switch(index)
    {
    case 0:     //TH
        ui->sensorHotAddrLbl->setText("Nr modułu TH");
        ui->sensorHotAddr->setMinimum(1);
        ui->sensorHotAddr->setMaximum(IO_MODULE_COUNT);
        ui->sensorHotRegNoLbl->setText("Nr kanału TH");
        ui->sensorHotRegNo->setMinimum(1);
        ui->sensorHotRegNo->setMaximum(4);
        break;
    case 1:     //CAN
        ui->sensorHotAddrLbl->setText("Adres CAN");
        ui->sensorHotAddr->setMinimum(1);
        ui->sensorHotAddr->setMaximum(255);
        ui->sensorHotRegNoLbl->setText("Nr meldunku");
        ui->sensorHotRegNo->setMinimum(0);
        ui->sensorHotRegNo->setMaximum(127);
        break;
    case 2:     //Modbus
        ui->sensorHotAddrLbl->setText("Adres Modbus");
        ui->sensorHotAddr->setMinimum(1);
        ui->sensorHotAddr->setMaximum(255);
        ui->sensorHotRegNoLbl->setText("Nr rejestru");
        ui->sensorHotRegNo->setMinimum(0);
        ui->sensorHotRegNo->setMaximum(65535);
        break;
    }
}

void MainWindow::on_blowSensTypeCmb_currentIndexChanged(int index)
{
    switch(index)
    {
    case 0:     //IO
        ui->blowSensorAddrLbl->setText("Nr modułu IO");
        ui->blowSensorAddr->setMinimum(1);
        ui->blowSensorAddr->setMaximum(IO_MODULE_COUNT);
        ui->blowSensorBitNoLbl->setText("Nr wejścia");
        ui->blowSensorBitNo->setMinimum(1);
        ui->blowSensorBitNo->setMaximum(20);
        ui->blowSensorRegNoLbl->setVisible(false);
        ui->blowSensorRegNo->setVisible(false);
        break;
    case 1:     //CAN
        ui->blowSensorAddrLbl->setText("Adres CAN");
        ui->blowSensorAddr->setMinimum(1);
        ui->blowSensorAddr->setMaximum(255);
        ui->blowSensorBitNoLbl->setText("Nr bitu");
        ui->blowSensorBitNo->setMinimum(1);
        ui->blowSensorBitNo->setMaximum(16);
        ui->blowSensorRegNoLbl->setVisible(true);
        ui->blowSensorRegNoLbl->setText("Nr meldunku");
        ui->blowSensorRegNo->setMinimum(0);
        ui->blowSensorRegNo->setMaximum(127);
        ui->blowSensorRegNo->setVisible(true);
        break;
    case 2:     //Modbus
        ui->blowSensorAddrLbl->setText("Adres Modbus");
        ui->blowSensorAddr->setMinimum(1);
        ui->blowSensorAddr->setMaximum(255);
        ui->blowSensorBitNoLbl->setText("Nr bitu");
        ui->blowSensorBitNo->setMinimum(1);
        ui->blowSensorBitNo->setMaximum(16);
        ui->blowSensorRegNoLbl->setVisible(true);
        ui->blowSensorRegNoLbl->setText("Nr rejestru");
        ui->blowSensorRegNo->setMinimum(0);
        ui->blowSensorRegNo->setMaximum(65535);
        ui->blowSensorRegNo->setVisible(true);
        break;
    }
}

void MainWindow::on_cirCount_valueChanged(int arg1)
{
    int rows = ui->circuitList->count();
    int i;
    QListWidgetItem *itm;
    QTextCodec *codec = QTextCodec::codecForName("ISO 8859-2");

    if((arg1 > rows) && (arg1 <= CIRCUIT_COUNT))
    {
        for(i = rows; i < arg1; i++)
        {
            if(circuit_cfg[i].name[0] == '\0')
            {
                ui->circuitList->addItem(QString("Obwód%1").arg(i + 1));
                strcpy(circuit_cfg[i].name, codec->fromUnicode(QString("Obwód%1").arg(i + 1)).data());
            }
            else
            {
                ui->circuitList->addItem(codec->toUnicode(circuit_cfg[i].name));
            }
        }
    }
    else if(arg1 < rows)
    {
        for(i = (rows - 1); i >= arg1; i--)
        {
            itm = ui->circuitList->takeItem(i);
            delete itm;
        }
    }
    ui->referenceCirNo->setMaximum(arg1);
}

void MainWindow::on_editCircuitList_clicked()
{
    int currentRow = ui->circuitList->currentRow();
    int i;
    QTextCodec *codec = QTextCodec::codecForName("ISO 8859-2");

    if(ui->editCircuitList->text() == "Zmień ustawienia")
    {
        ui->editCircuitList->setText("Zapisz zmiany");
        ui->circuitList->setEnabled(false);
        ui->cirNameEdit->setEnabled(true);
        ui->cirActiveChk->setEnabled(true);
        //ui->cirReferenceChk->setEnabled(true);
        ui->cirTypeCmb->setEnabled(true);
        ui->cirGroupNo->setEnabled(true);
        ui->cirWeatherAutomNo->setEnabled(true);
        ui->l1Frame->setEnabled(true);
        ui->l2Frame->setEnabled(true);
        ui->l3Frame->setEnabled(true);
        ui->cirRelayFrame->setEnabled(true);
        ui->cirRelayConfFrame->setEnabled(true);
    }
    else
    {
        memset(&circuit_cfg[currentRow], 0, sizeof(circuit_cfg_t));
        strcpy(circuit_cfg[currentRow].name, codec->fromUnicode(ui->cirNameEdit->text()).data());
        ui->circuitList->item(currentRow)->setText(ui->cirNameEdit->text());
        circuit_cfg[currentRow].active = ui->cirActiveChk->isChecked();
        circuit_cfg[currentRow].reference = ui->cirReferenceChk->isChecked();
        circuit_cfg[currentRow].type = ui->cirTypeCmb->currentIndex();
        circuit_cfg[currentRow].group_id = ui->cirGroupNo->value();
        circuit_cfg[currentRow].weather_autom_id = ui->cirWeatherAutomNo->value();

        circuit_cfg[currentRow].phase_cfg[0].active = ui->l1ActiveChk->isChecked();
        circuit_cfg[currentRow].phase_cfg[0].conf_input.active = ui->l1ConfActiveChk->isChecked();
        circuit_cfg[currentRow].phase_cfg[0].conf_input.module_id = ui->l1ConfIOMod->value();
        circuit_cfg[currentRow].phase_cfg[0].conf_input.bit_no = ui->l1ConfBitNo->value();

        circuit_cfg[currentRow].phase_cfg[0].cvm_id = ui->l1CvmId->value();
        circuit_cfg[currentRow].phase_cfg[0].cvm_ch_id = ui->l1CvmCh->value();

        circuit_cfg[currentRow].phase_cfg[0].p_nom = ui->p1Nominal->value() * 10;
        circuit_cfg[currentRow].phase_cfg[0].p_tol = ui->p1Tol->value() * 10;

        circuit_cfg[currentRow].phase_cfg[1].active = ui->l2ActiveChk->isChecked();
        circuit_cfg[currentRow].phase_cfg[1].conf_input.active = ui->l2ConfActiveChk->isChecked();
        circuit_cfg[currentRow].phase_cfg[1].conf_input.module_id = ui->l2ConfIOMod->value();
        circuit_cfg[currentRow].phase_cfg[1].conf_input.bit_no = ui->l2ConfBitNo->value();

        circuit_cfg[currentRow].phase_cfg[1].cvm_id = ui->l2CvmId->value();
        circuit_cfg[currentRow].phase_cfg[1].cvm_ch_id = ui->l2CvmCh->value();

        circuit_cfg[currentRow].phase_cfg[1].p_nom = ui->p2Nominal->value() * 10;
        circuit_cfg[currentRow].phase_cfg[1].p_tol = ui->p2Tol->value() * 10;

        circuit_cfg[currentRow].phase_cfg[2].active = ui->l3ActiveChk->isChecked();
        circuit_cfg[currentRow].phase_cfg[2].conf_input.active = ui->l3ConfActiveChk->isChecked();
        circuit_cfg[currentRow].phase_cfg[2].conf_input.module_id = ui->l3ConfIOMod->value();
        circuit_cfg[currentRow].phase_cfg[2].conf_input.bit_no = ui->l3ConfBitNo->value();

        circuit_cfg[currentRow].phase_cfg[2].cvm_id = ui->l3CvmId->value();
        circuit_cfg[currentRow].phase_cfg[2].cvm_ch_id = ui->l3CvmCh->value();

        circuit_cfg[currentRow].phase_cfg[2].p_nom = ui->p3Nominal->value() * 10;
        circuit_cfg[currentRow].phase_cfg[2].p_tol = ui->p3Tol->value() * 10;

        circuit_cfg[currentRow].relay.active = 1;
        circuit_cfg[currentRow].relay.module_id = ui->cirRelIOMod->value();
        circuit_cfg[currentRow].relay.bit_no = ui->cirRelBitNo->value();
        circuit_cfg[currentRow].rel_conf.active = 1;
        circuit_cfg[currentRow].rel_conf.module_id = ui->cirRelConfIOMod->value();
        circuit_cfg[currentRow].rel_conf.bit_no = ui->cirRelConfBitNo->value();

        if(checkIoMod(circuit_cfg[currentRow].rel_conf.module_id, 0, "Potwierdzenie stycznika") == 1)
            return;

        if(checkIoMod(circuit_cfg[currentRow].relay.module_id, 1, "Sterowanie stycznikem") == 1)
            return;

        for(i = 0; i < 3; i++)
        {
            if((circuit_cfg[currentRow].phase_cfg[i].active != 0) &&
                    (circuit_cfg[currentRow].phase_cfg[i].conf_input.active != 0))
            {
                if(checkIoMod(circuit_cfg[currentRow].phase_cfg[i].conf_input.module_id, 0, QString("Potwierdzenie zabezpieczenia fazy L%1").arg(i + 1)) == 1)
                    return;

                if(checkIoMod(circuit_cfg[currentRow].phase_cfg[i].cvm_id, 3, QString("Moduł CVM fazy L%1").arg(i + 1)) == 1)
                    return;
            }
        }

        ui->editCircuitList->setText("Zmień ustawienia");
        ui->circuitList->setEnabled(true);
        ui->cirNameEdit->setEnabled(false);
        ui->cirActiveChk->setEnabled(false);
        ui->cirReferenceChk->setEnabled(false);
        ui->cirTypeCmb->setEnabled(false);
        ui->cirGroupNo->setEnabled(false);
        ui->cirWeatherAutomNo->setEnabled(false);
        ui->l1Frame->setEnabled(false);
        ui->l2Frame->setEnabled(false);
        ui->l3Frame->setEnabled(false);
        ui->cirRelayFrame->setEnabled(false);
        ui->cirRelayConfFrame->setEnabled(false);
    }
}

void MainWindow::on_circuitList_currentRowChanged(int currentRow)
{
    setCircuitCfg(currentRow);
}

void MainWindow::on_modbusSlaveMediumCmb_currentIndexChanged(int index)
{
    switch(index)
    {
    case 0:
        ui->modbusSlavePort->setMinimum(1);
        ui->modbusSlavePort->setMaximum(6);
        break;
    case 1:
        ui->modbusSlavePort->setMinimum(0);
        ui->modbusSlavePort->setMaximum(65535);
        break;
    }
}

void MainWindow::on_referenceCirNo_valueChanged(int arg1)
{
    int i;

    weather_autom_cfg[ui->weatherAutomList->currentRow()].referenceCircuitNo = arg1;

    for(i = 0; i < CIRCUIT_COUNT; i++)
    {
        circuit_cfg[i].reference = 0;
    }

    for(i = 0; i < WEATHER_AUTOM_COUNT; i++)
    {
        circuit_cfg[weather_autom_cfg[i].referenceCircuitNo - 1].reference = 1;
    }

    setWeatherAutomCfg(ui->weatherAutomList->currentRow());
    setCircuitCfg(ui->circuitList->currentRow());
}
