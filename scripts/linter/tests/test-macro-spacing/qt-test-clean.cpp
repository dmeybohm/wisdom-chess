class ExampleTest : public QObject
{
    Q_OBJECT

private slots:
    void example()
    {
        QFETCH( QString, name );
        QVERIFY( value );
        QVERIFY2( value, "message" );
        QCOMPARE( model.rowCount ({}), 32 );
        QTRY_COMPARE( spy.count(), 1 );
        QVERIFY_THROWS_EXCEPTION( wisdom::Error, MaxDepth { 0 } );
        QCOMPARE(
            first,
            second
        );
    }
};

QTEST_GUILESS_MAIN( ExampleTest )
