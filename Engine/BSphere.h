// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#pragma once

class BSphere
{
  float m_Center[3];
  float m_Radius;

public:
  BSphere()
  {
    m_Center[0] = m_Center[1] = m_Center[2] = 0.0f;
    m_Radius = 0.0f;
  }

  inline float z() const
  {
    return m_Center[2];
  }
  
  inline float radius() const
  {
    return m_Radius;
  }
  
  inline BSphere(float pos[3], float rad)
  {
    m_Center[0] = pos[0];
    m_Center[1] = pos[1];
    m_Center[2] = pos[2];
    m_Radius = rad;
  }
};
