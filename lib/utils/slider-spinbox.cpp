#include "slider-spinbox.hpp"
#include <QHBoxLayout>

namespace advss {

SliderSpinBox::SliderSpinBox(double min, double max, const QString &label,
			     const QString &description,
			     bool descriptionAsTooltip, QWidget *parent)
	: QWidget(parent),
	  _spinBox(new VariableDoubleSpinBox()),
	  _slider(new QSlider())
{
	_slider->setOrientation(Qt::Horizontal);
	_slider->setRange(min * _scale, max * _scale);
	_spinBox->setMinimum(min);
	_spinBox->setMaximum(max);
	_spinBox->setDecimals(5);
	// Default QDoubleSpinBox step is 1.0, which jumps 1.00000 ↔ 0.00000
	// in a 0–1 threshold field. Match the decimal places instead.
	_spinBox->SpinBox()->setSingleStep(0.00001);

	connect(_slider, SIGNAL(valueChanged(int)), this,
		SLOT(SliderValueChanged(int)));
	QWidget::connect(
		_spinBox,
		SIGNAL(NumberVariableChanged(const NumberVariable<double> &)),
		this,
		SLOT(SpinBoxValueChanged(const NumberVariable<double> &)));

	auto mainLayout = new QVBoxLayout();
	auto sliderLayout = new QHBoxLayout();
	if (!label.isEmpty()) {
		sliderLayout->addWidget(new QLabel(label));
	}
	sliderLayout->addWidget(_spinBox);
	sliderLayout->addWidget(_slider);
	mainLayout->addLayout(sliderLayout);
	if (!description.isEmpty()) {
		if (descriptionAsTooltip) {
			setToolTip(description);
		} else {
			mainLayout->addWidget(new QLabel(description));
		}
	}
	mainLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(mainLayout);
}

void SliderSpinBox::SetDoubleValue(double value)
{
	NumberVariable<double> temp = value;
	SetDoubleValue(temp);
}

void SliderSpinBox::SetDoubleValue(const NumberVariable<double> &value)
{
	const QSignalBlocker b1(_slider);
	const QSignalBlocker b2(_spinBox);
	const QSignalBlocker b3(_spinBox->SpinBox());
	_slider->setValue(qRound(value.GetFixedValue() * _scale));
	_spinBox->SetValue(value);
	SetVisibility(value);
}

void SliderSpinBox::SpinBoxValueChanged(const NumberVariable<double> &value)
{
	if (value.IsFixedType()) {
		// Block slider signals so a coarse slider step cannot write
		// back into the spin box (e.g. 0.98765 → 0.98000).
		const QSignalBlocker b(_slider);
		_slider->setValue(qRound(value.GetFixedValue() * _scale));
	}
	SetVisibility(value);
	emit DoubleValueChanged(value);
}

void SliderSpinBox::SliderValueChanged(int value)
{
	NumberVariable<double> doubleValue = value / _scale;
	const QSignalBlocker b(_spinBox->SpinBox());
	_spinBox->SetValue(doubleValue);
	emit DoubleValueChanged(doubleValue);
}

void SliderSpinBox::SetVisibility(const NumberVariable<double> &value)
{
	_slider->setVisible(value.IsFixedType());
}

} // namespace advss
