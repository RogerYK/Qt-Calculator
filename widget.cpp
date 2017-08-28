#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->lineEdit->setText("0");
    result = 0.0;
    waitForOperand = true;

    connectSlots();
}

Widget::~Widget()
{
    delete ui;
}
bool Widget::calculate(double operand, QString pendingOperator)
{
    if(pendingOperator == "+")
    {
        result += operand;
    }
    else if(pendingOperator == "-")
    {
        result -= operand;
    }
    else if(pendingOperator == "*")
    {
        result *= operand;
    }
    else if(pendingOperator == "/")
    {
        if(operand == 0.0)
            return false;
        result /= operand;
    }
    return true;
}

void Widget::abortOperation()
{
    result = 0.0;
    pendingOperator.clear();
    ui->lineEdit->setText("0");
    waitForOperand = true;
    QMessageBox::warning(this, "运算错误", "除数不能为零");
}

void Widget::digitClicked()
{
    QPushButton *digitBtn = static_cast<QPushButton*>(sender());
    QString value = digitBtn->text();
    if(ui->lineEdit->text() == "0" && value == "0")
        return;
    if(waitForOperand)
    {
        ui->lineEdit->setText(value);
        waitForOperand = false;
    }
    else
    {
        ui->lineEdit->setText(ui->lineEdit->text() + value);
    }
}

void Widget::on_clearBtn_clicked()
{
    //将当前显示的数归零
    ui->lineEdit->setText("0");
    waitForOperand = true;
}

void Widget::on_clearAllBtn_clicked()
{
    //将当前显示的数据归零，并将之前保存的数据运算清除
    ui->lineEdit->setText("0");
    waitForOperand = true;
    result = 0.0;
    pendingOperator.clear();
}

void Widget::on_equalBtn_clicked()
{
    double operand = ui->lineEdit->text().toDouble();
    if(pendingOperator.isEmpty())
        return;
    if(!calculate(operand, pendingOperator))
    {
        abortOperation();
        return;
    }
    ui->lineEdit->setText(QString::number(result));
    pendingOperator.clear();
    result = 0.0;
    waitForOperand = true;
}

void Widget::on_signBtn_clicked()
{
    QString text = ui->lineEdit->text();
    double value = text.toDouble();
    if(value > 0)
    {
        text.prepend('-');
    }
    else if(value < 0)
    {
        text.remove(0, 1);
    }
    ui->lineEdit->setText(text);
}

void Widget::operatorClicked()
{
    QPushButton *clickedBtn = qobject_cast<QPushButton *>(sender());
    QString clickedOperator = clickedBtn->text();
    double operand = ui->lineEdit->text().toDouble();
    if(!pendingOperator.isEmpty())
    {
        if(!calculate(operand, pendingOperator))
        {
            abortOperation();
            return;
        }
        ui->lineEdit->setText(QString::number(result));
    }
    else
    {
        result = operand;
    }
    pendingOperator = clickedOperator;
    waitForOperand = true;
}


void Widget::on_pointBtn_clicked()
{
    if (waitForOperand)
        ui->lineEdit->setText("0");
    if (!ui->lineEdit->text().contains('.'))
        ui->lineEdit->setText(ui->lineEdit->text() + ".");
    waitForOperand = false;
}

void Widget::connectSlots()
{
    connect(ui->digitBnt0, &QPushButton::clicked, this, &Widget::digitClicked);
    connect(ui->digitBtn1, &QPushButton::clicked, this, &Widget::digitClicked);
    connect(ui->digitBtn2, &QPushButton::clicked, this, &Widget::digitClicked);
    connect(ui->digitBtn3, &QPushButton::clicked, this, &Widget::digitClicked);
    connect(ui->digitBtn4, &QPushButton::clicked, this, &Widget::digitClicked);
    connect(ui->digitBtn5, &QPushButton::clicked, this, &Widget::digitClicked);
    connect(ui->digitBtn6, &QPushButton::clicked, this, &Widget::digitClicked);
    connect(ui->digitBtn7, &QPushButton::clicked, this, &Widget::digitClicked);
    connect(ui->digitBtn8, &QPushButton::clicked, this, &Widget::digitClicked);
    connect(ui->digitBtn9, &QPushButton::clicked, this, &Widget::digitClicked);
    connect(ui->mulBtn, &QPushButton::clicked, this, &Widget::operatorClicked);
    connect(ui->divisionBtn, &QPushButton::clicked, this, &Widget::operatorClicked);
    connect(ui->addBtn, &QPushButton::clicked, this, &Widget::operatorClicked);
    connect(ui->subtractionBtn, &QPushButton::clicked, this, &Widget::operatorClicked);
}
