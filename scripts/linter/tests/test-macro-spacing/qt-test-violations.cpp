void example()
{
    QVERIFY(value);
    QCOMPARE (first, second);
    QVERIFY2( value, "message");
}

QTEST_MAIN(ExampleTest)
