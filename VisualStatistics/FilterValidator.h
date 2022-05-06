#ifndef FILTERVALIDATOR_H
#define FILTERVALIDATOR_H

#include <QValidator>

class FilterValidator : public QValidator
{
public:
    explicit FilterValidator(QObject *parent);

    virtual void fixup(QString &input) const;
    virtual State validate(QString &input, int &pos) const;
};

#endif // FILTERVALIDATOR_H
