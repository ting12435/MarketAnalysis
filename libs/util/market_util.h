#ifndef MARKET_UTIL_H
#define MARKET_UTIL_H

enum Market { ALL, TSE, OTC };

enum security_type { 
					Stock, 
					TDR, 
					BC /* Beneficiary certificate */, 
					ETF, 
					BS /* Beneficiary Securities */, 
					FullCashDeliveryStock, 
					Warrant, 
					ConvertibleBonds /* Convertible bonds */,
					PreferredSharesWithWarrants /* 附認股權特別股 */,
					CorporateBondsWithWarrants/* 附認股權公司債 */,
					StockOptionCertificates /* 認股權憑證 */,
					CorporateBond /* 公司債 */,
					ETN 
};

#endif /* MARKET_UTIL_H */