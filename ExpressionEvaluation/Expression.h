#pragma once

#include <memory>

class Expression
{
public:
   virtual double Evaluate() = 0;
};

 class Literal sealed : public Expression
{
private:
   double m_value;

public:
   explicit Literal( const double value ) :
      m_value( value )
   { }

   double Evaluate() override
   { return m_value; }
};

class BinaryExpression : public Expression
{
protected:
   std::shared_ptr< Expression > m_left, m_right;

public:
   BinaryExpression( const std::shared_ptr< Expression > expressionLeft, const std::shared_ptr< Expression > expressionRight ) :
      m_left( expressionLeft ),
      m_right( expressionRight )
   { }
};


#pragma region Specific operator implementations
class AddExpression sealed : BinaryExpression
{
public:
   AddExpression( const std::shared_ptr< Expression > expressionLeft, const std::shared_ptr< Expression > expressionRight ) :
      BinaryExpression( expressionLeft, expressionRight )
   { }

   double Evaluate() override
   { return m_left->Evaluate() + m_right->Evaluate(); }
};

class SubExpression sealed : BinaryExpression
{
public:
   SubExpression( const std::shared_ptr< Expression > expressionLeft, const std::shared_ptr< Expression > expressionRight ) :
      BinaryExpression( expressionLeft, expressionRight )
   { }

   double Evaluate() override
   { return m_left->Evaluate() - m_right->Evaluate(); }
};

class MultExpression sealed : BinaryExpression
{
public:
   MultExpression( const std::shared_ptr< Expression > expressionLeft, const std::shared_ptr< Expression > expressionRight ) :
      BinaryExpression( expressionLeft, expressionRight )
   { }

   double Evaluate() override
   { return m_left->Evaluate() * m_right->Evaluate(); }
};

class DivExpression sealed : BinaryExpression
{
public:
   double Evaluate() override
   { return m_left->Evaluate() / m_right->Evaluate(); }
};
#pragma endregion