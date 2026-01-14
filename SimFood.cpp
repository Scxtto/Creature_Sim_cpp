#include "SimFood.h"

Food::Food(int id, double x, double y, double energyContent)
    : m_id(id)
    , m_x(x)
    , m_y(y)
    , m_energyContent(energyContent)
{
}

void Food::update()
{
    m_duration -= 1;
    if (m_duration <= 0) {
        m_consumed = true;
        m_energyContent = 0.0;
    }
}

void Food::markConsumed()
{
    m_consumed = true;
    m_energyContent = 0.0;
}
