auto a = "escaped \" quote";
auto b = '\'';
auto c = u8"prefixed" L'w' U"wide";
auto d = R"delim(raw )" still raw
over lines)delim";
auto e = "text"s + 12_km;
auto f = "spliced \
string";
auto g = 'unterminated
int h;
