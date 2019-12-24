#ifndef __TUSHARECPP__
#define __TUSHARECPP__

#include "data_container.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include<map>

namespace ts {
	using namespace std;

	template<typename data_store=data_container<vector> >
	class pro_api {
	private:
		map<string, string> _fields_map;
		map<string, string> _types_map;

		string host = "api.waditu.com";
		string port = "80";
		string token;

		boost::asio::io_context ioc;
		boost::asio::ip::tcp::resolver resolver{ ioc };
		boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> results;
		boost::asio::ip::tcp::socket sockets{ ioc };
		boost::beast::http::request<boost::beast::http::string_body> req;
		boost::beast::http::response<boost::beast::http::dynamic_body> res;
		boost::beast::flat_buffer buffer;

		ptree base_api_req_tree;

		void connect_socket() {
			results = resolver.resolve(host, port);
			boost::asio::connect(sockets, results.begin(), results.end());
			req.method(boost::beast::http::verb::post);
			req.target("/");
			req.set(boost::beast::http::field::host, host);
			req.set(boost::beast::http::field::content_type, "application/json");
			req.keep_alive(true);
		};


		void initialize_base_api_req_tree() {
			base_api_req_tree.put("token", token);
		};

		ptree gen_api_req_tree(const string& apiname, const ptree& paramsjson) {
			ptree req_tree = base_api_req_tree;
			req_tree.put("api_name", apiname);
			req_tree.put("fields", _fields_map[apiname]);
			req_tree.put_child("params", paramsjson);

			return req_tree;
		};

		void fetch_data(ptree& req_tree) {
			ostringstream buf;
			write_json(buf, req_tree, false);
			string reqstr = buf.str();
			req.body() = reqstr;
			req.prepare_payload();

			cout << reqstr << endl;
			//Get response from server.
			try {
				boost::beast::http::write(sockets, req);
				res = {};
				buffer.consume(buffer.size());
				boost::beast::http::read(sockets, buffer, res);

			}
			catch (...) {
				connect_socket();
			};

			
		};

		data_store process(const string& apiname, const ptree& parmtree) {
			ptree req_tree = gen_api_req_tree(apiname, parmtree);
			fetch_data(req_tree);
			auto data_tree=parse_data();
			cout << "begin construct data container." << endl;
			auto dt=data_container(data_tree,_types_map[apiname]);
			return dt;
		};

		ptree parse_data() {
			ptree datajson;
			stringstream datastr;
			datastr << boost::beast::buffers_to_string(res.body().data());
			read_json(datastr, datajson);
			return datajson.get_child("data.items.");
		};

		void initialize_data_map() {
			_fields_map={{"stock_basic", "ts_code,symbol,name,area,industry,fullname,enname,market,exchange,curr_type,list_status,list_date,delist_date,is_hs"},
				{"trade_cal", "exchange,cal_date,is_open,pretrade_date"},
				{"namechange", "ts_code,name,start_date,end_date,ann_date,change_reason"},
				{"hs_const", "ts_code,hs_type,in_date,out_date,is_new"},
				{"stock_company", "ts_code,exchange,chairman,manager,secretary,reg_capital,setup_date,province,city,introduction,website,email,office,employees,main_business,business_scope"},
				{"stk_managers", "ts_code,ann_date,name,gender,lev,title,edu,national,birthday,begin_date,end_date,resume"},
				{"stk_rewards", "ts_code,ann_date,end_date,name,title,reward,hold_vol"},
				{"new_share", "ts_code,sub_code,name,ipo_date,issue_date,amount,market_amount,price,pe,limit_amount,funds,ballot"},
				{"daily", "ts_code,trade_date,open,high,low,close,pre_close,change,pct_chg,vol,amount"},
				{"weekly", "ts_code,trade_date,close,open,high,low,pre_close,change,pct_chg,vol,amount"},
				{"monthly", "ts_code,trade_date,close,open,high,low,pre_close,change,pct_chg,vol,amount"},
				{"adj_factor", "ts_code,trade_date,adj_factor"},
				{"suspend", "ts_code,suspend_date,resume_date,ann_date,suspend_reason,reason_type"},
				{"daily_basic", "ts_code,trade_date,close,turnover_rate,turnover_rate_f,volume_ratio,pe,pe_ttm,pb,ps,ps_ttm,dv_ratio,dv_ttm,total_share,float_share,free_share,total_mv,circ_mv"},
				{"moneyflow", "ts_code,trade_date,buy_sm_vol,buy_sm_amount,sell_sm_vol,sell_sm_amount,buy_md_vol,buy_md_amount,sell_md_vol,sell_md_amount,buy_lg_vol,buy_lg_amount,sell_lg_vol,sell_lg_amount,buy_elg_vol,buy_elg_amount,sell_elg_vol,sell_elg_amount,net_mf_vol,net_mf_amount"},
				{"stk_limit", "trade_date,ts_code,pre_close,up_limit,down_limit"},
				{"limit_list", "trade_date,ts_code,name,close,pct_chg,amp,fc_ratio,fl_ratio,fd_amount,first_time,last_time,open_times,strth,limit"},
				{"moneyflow_hsgt", "trade_date,ggt_ss,ggt_sz,hgt,sgt,north_money,south_money"},
				{"hsgt_top10", "trade_date,ts_code,name,close,change,rank,market_type,amount,net_amount,buy,sell"},
				{"hk_hold", "code,trade_date,ts_code,name,vol,ratio,exchange"},
				{"ggt_daily", "trade_date,buy_amount,buy_volume,sell_amount,sell_volume"},
				{"ggt_monthly", "month,day_buy_amt,day_buy_vol,day_sell_amt,day_sell_vol,total_buy_amt,total_buy_vol,total_sell_amt,total_sell_vol"},
				{"balancesheet", "ts_code,ann_date,f_ann_date,end_date,report_type,comp_type,total_share,cap_rese,undistr_porfit,surplus_rese,special_rese,money_cap,trad_asset,notes_receiv,accounts_receiv,oth_receiv,prepayment,div_receiv,int_receiv,inventories,amor_exp,nca_within_1y,sett_rsrv,loanto_oth_bank_fi,premium_receiv,reinsur_receiv,reinsur_res_receiv,pur_resale_fa,oth_cur_assets,total_cur_assets,fa_avail_for_sale,htm_invest,lt_eqt_invest,invest_real_estate,time_deposits,oth_assets,lt_rec,fix_assets,cip,const_materials,fixed_assets_disp,produc_bio_assets,oil_and_gas_assets,intan_assets,r_and_d,goodwill,lt_amor_exp,defer_tax_assets,decr_in_disbur,oth_nca,total_nca,cash_reser_cb,depos_in_oth_bfi,prec_metals,deriv_assets,rr_reins_une_prem,rr_reins_outstd_cla,rr_reins_lins_liab,rr_reins_lthins_liab,refund_depos,ph_pledge_loans,refund_cap_depos,indep_acct_assets,client_depos,client_prov,transac_seat_fee,invest_as_receiv,total_assets,lt_borr,st_borr,cb_borr,depos_ib_deposits,loan_oth_bank,trading_fl,notes_payable,acct_payable,adv_receipts,sold_for_repur_fa,comm_payable,payroll_payable,taxes_payable,int_payable,div_payable,oth_payable,acc_exp,deferred_inc,st_bonds_payable,payable_to_reinsurer,rsrv_insur_cont,acting_trading_sec,acting_uw_sec,non_cur_liab_due_1y,oth_cur_liab,total_cur_liab,bond_payable,lt_payable,specific_payables,estimated_liab,defer_tax_liab,defer_inc_non_cur_liab,oth_ncl,total_ncl,depos_oth_bfi,deriv_liab,depos,agency_bus_liab,oth_liab,prem_receiv_adva,depos_received,ph_invest,reser_une_prem,reser_outstd_claims,reser_lins_liab,reser_lthins_liab,indept_acc_liab,pledge_borr,indem_payable,policy_div_payable,total_liab,treasury_share,ordin_risk_reser,forex_differ,invest_loss_unconf,minority_int,total_hldr_eqy_exc_min_int,total_hldr_eqy_inc_min_int,total_liab_hldr_eqy,lt_payroll_payable,oth_comp_income,oth_eqt_tools,oth_eqt_tools_p_shr,lending_funds,acc_receivable,st_fin_payable,payables,hfs_assets,hfs_sales,update_flag"},
				{"cashflow", "ts_code,ann_date,f_ann_date,end_date,comp_type,report_type,net_profit,finan_exp,c_fr_sale_sg,recp_tax_rends,n_depos_incr_fi,n_incr_loans_cb,n_inc_borr_oth_fi,prem_fr_orig_contr,n_incr_insured_dep,n_reinsur_prem,n_incr_disp_tfa,ifc_cash_incr,n_incr_disp_faas,n_incr_loans_oth_bank,n_cap_incr_repur,c_fr_oth_operate_a,c_inf_fr_operate_a,c_paid_goods_s,c_paid_to_for_empl,c_paid_for_taxes,n_incr_clt_loan_adv,n_incr_dep_cbob,c_pay_claims_orig_inco,pay_handling_chrg,pay_comm_insur_plcy,oth_cash_pay_oper_act,st_cash_out_act,n_cashflow_act,oth_recp_ral_inv_act,c_disp_withdrwl_invest,c_recp_return_invest,n_recp_disp_fiolta,n_recp_disp_sobu,stot_inflows_inv_act,c_pay_acq_const_fiolta,c_paid_invest,n_disp_subs_oth_biz,oth_pay_ral_inv_act,n_incr_pledge_loan,stot_out_inv_act,n_cashflow_inv_act,c_recp_borrow,proc_issue_bonds,oth_cash_recp_ral_fnc_act,stot_cash_in_fnc_act,free_cashflow,c_prepay_amt_borr,c_pay_dist_dpcp_int_exp,incl_dvd_profit_paid_sc_ms,oth_cashpay_ral_fnc_act,stot_cashout_fnc_act,n_cash_flows_fnc_act,eff_fx_flu_cash,n_incr_cash_cash_equ,c_cash_equ_beg_period,c_cash_equ_end_period,c_recp_cap_contrib,incl_cash_rec_saims,uncon_invest_loss,prov_depr_assets,depr_fa_coga_dpba,amort_intang_assets,lt_amort_deferred_exp,decr_deferred_exp,incr_acc_exp,loss_disp_fiolta,loss_scr_fa,loss_fv_chg,invest_loss,decr_def_inc_tax_assets,incr_def_inc_tax_liab,decr_inventories,decr_oper_payable,incr_oper_payable,others,im_net_cashflow_oper_act,conv_debt_into_cap,conv_copbonds_due_within_1y,fa_fnc_leases,end_bal_cash,beg_bal_cash,end_bal_cash_equ,beg_bal_cash_equ,im_n_incr_cash_equ,update_flag"},
				{"forecast", "ts_code,ann_date,end_date,type,p_change_min,p_change_max,net_profit_min,net_profit_max,last_parent_net,first_ann_date,summary,change_reason"},
				{"express", "ts_code,ann_date,end_date,revenue,operate_profit,total_profit,n_income,total_assets,total_hldr_eqy_exc_min_int,diluted_eps,diluted_roe,yoy_net_profit,bps,yoy_sales,yoy_op,yoy_tp,yoy_dedu_np,yoy_eps,yoy_roe,growth_assets,yoy_equity,growth_bps,or_last_year,op_last_year,tp_last_year,np_last_year,eps_last_year,open_net_assets,open_bps,perf_summary,is_audit,remark"},
				{"dividend", "ts_code,end_date,ann_date,div_proc,stk_div,stk_bo_rate,stk_co_rate,cash_div,cash_div_tax,record_date,ex_date,pay_date,div_listdate,imp_ann_date,base_date,base_share"},
				{"fina_indicator", "ts_code,ann_date,end_date,eps,dt_eps,total_revenue_ps,revenue_ps,capital_rese_ps,surplus_rese_ps,undist_profit_ps,extra_item,profit_dedt,gross_margin,current_ratio,quick_ratio,cash_ratio,invturn_days,arturn_days,inv_turn,ar_turn,ca_turn,fa_turn,assets_turn,op_income,valuechange_income,interst_income,daa,ebit,ebitda,fcff,fcfe,current_exint,noncurrent_exint,interestdebt,netdebt,tangible_asset,working_capital,networking_capital,invest_capital,retained_earnings,diluted2_eps,bps,ocfps,retainedps,cfps,ebit_ps,fcff_ps,fcfe_ps,netprofit_margin,grossprofit_margin,cogs_of_sales,expense_of_sales,profit_to_gr,saleexp_to_gr,adminexp_of_gr,finaexp_of_gr,impai_ttm,gc_of_gr,op_of_gr,ebit_of_gr,roe,roe_waa,roe_dt,roa,npta,roic,roe_yearly,roa2_yearly,roe_avg,opincome_of_ebt,investincome_of_ebt,n_op_profit_of_ebt,tax_to_ebt,dtprofit_to_profit,salescash_to_or,ocf_to_or,ocf_to_opincome,capitalized_to_da,debt_to_assets,assets_to_eqt,dp_assets_to_eqt,ca_to_assets,nca_to_assets,tbassets_to_totalassets,int_to_talcap,eqt_to_talcapital,currentdebt_to_debt,longdeb_to_debt,ocf_to_shortdebt,debt_to_eqt,eqt_to_debt,eqt_to_interestdebt,tangibleasset_to_debt,tangasset_to_intdebt,tangibleasset_to_netdebt,ocf_to_debt,ocf_to_interestdebt,ocf_to_netdebt,ebit_to_interest,longdebt_to_workingcapital,ebitda_to_debt,turn_days,roa_yearly,roa_dp,fixed_assets,profit_prefin_exp,non_op_profit,op_to_ebt,nop_to_ebt,ocf_to_profit,cash_to_liqdebt,cash_to_liqdebt_withinterest,op_to_liqdebt,op_to_debt,roic_yearly,total_fa_trun,profit_to_op,q_opincome,q_investincome,q_dtprofit,q_eps,q_netprofit_margin,q_gsprofit_margin,q_exp_to_sales,q_profit_to_gr,q_saleexp_to_gr,q_adminexp_to_gr,q_finaexp_to_gr,q_impair_to_gr_ttm,q_gc_to_gr,q_op_to_gr,q_roe,q_dt_roe,q_npta,q_opincome_to_ebt,q_investincome_to_ebt,q_dtprofit_to_profit,q_salescash_to_or,q_ocf_to_sales,q_ocf_to_or,basic_eps_yoy,dt_eps_yoy,cfps_yoy,op_yoy,ebt_yoy,netprofit_yoy,dt_netprofit_yoy,ocf_yoy,roe_yoy,bps_yoy,assets_yoy,eqt_yoy,tr_yoy,or_yoy,q_gr_yoy,q_gr_qoq,q_sales_yoy,q_sales_qoq,q_op_yoy,q_op_qoq,q_profit_yoy,q_profit_qoq,q_netprofit_yoy,q_netprofit_qoq,equity_yoy,rd_exp,update_flag"},
				{"fina_audit", "ts_code,ann_date,end_date,audit_result,audit_fees,audit_agency,audit_sign"},
				{"fina_mainbz", "ts_code,end_date,bz_item,bz_sales,bz_profit,bz_cost,curr_type,update_flag"},
				{"disclosure_date", "ts_code,ann_date,end_date,pre_date,actual_date,modify_date"},
				{"ggt_top10", "trade_date,ts_code,name,close,p_change,rank,market_type,amount,net_amount,sh_amount,sh_net_amount,sh_buy,sh_sell,sz_amount,sz_net_amount,sz_buy,sz_sell"},
				{"margin", "trade_date,exchange_id,rzye,rzmre,rzche,rqye,rqmcl,rzrqye,rqyl"},
				{"margin_detail", "trade_date,ts_code,name,rzye,rqye,rzmre,rqyl,rzche,rqchl,rqmcl,rzrqye"},
				{"top10_holders", "ts_code,ann_date,end_date,holder_name,hold_amount,hold_ratio"},
				{"top10_floatholders", "ts_code,ann_date,end_date,holder_name,hold_amount"},
				{"top_list", "trade_date,ts_code,name,close,pct_change,turnover_rate,amount,l_sell,l_buy,l_amount,net_amount,net_rate,amount_rate,float_values,reason"},
				{"top_inst", "trade_date,ts_code,exalter,buy,buy_rate,sell,sell_rate,net_buy"},
				{"pledge_stat", "ts_code,end_date,pledge_count,unrest_pledge,rest_pledge,total_share,pledge_ratio"},
				{"pledge_detail", "ts_code,ann_date,holder_name,pledge_amount,start_date,end_date,is_release,release_date,pledgor,holding_amount,pledged_amount,p_total_ratio,h_total_ratio,is_buyback"},
				{"repurchase", "ts_code,ann_date,end_date,proc,exp_date,vol,amount,high_limit,low_limit"},
				{"concept", "code,name,src"},
				{"concept_detail", "id,concept_name,ts_code,name,in_date,out_date"},
				{"share_float", "ts_code,ann_date,float_date,float_share,float_ratio,holder_name,share_type"},
				{"block_trade", "ts_code,trade_date,price,vol,amount,buyer,seller"},
				{"stk_account", "date,weekly_new,total,weekly_hold,weekly_trade"},
				{"stk_account_old", "date,new_sh,new_sz,active_sh,active_sz,total_sh,total_sz,trade_sh,trade_sz"},
				{"stk_holdernumber", "ts_code,ann_date,end_date,holder_num"},
				{"stk_holdertrade", "ts_code,ann_date,holder_name,holder_type,in_de,change_vol,change_ratio,after_share,after_ratio,avg_price,total_share,begin_date,close_date"},
				{"index_basic", "ts_code,name,fullname,market,publisher,index_type,category,base_date,base_point,list_date,weight_rule,desc,exp_date"},
				{"index_daily", "ts_code,trade_date,close,open,high,low,pre_close,change,pct_chg,vol,amount"},
				{"index_weekly", "ts_code,trade_date,close,open,high,low,pre_close,change,pct_chg,vol,amount"},
				{"index_monthly", "ts_code,trade_date,close,open,high,low,pre_close,change,pct_chg,vol,amount"},
				{"index_weight", "index_code,con_code,trade_date,weight"},
				{"index_dailybasic", "ts_code,trade_date,total_mv,float_mv,total_share,float_share,free_share,turnover_rate,turnover_rate_f,pe,pe_ttm,pb"},
				{"index_classify", "index_code,industry_name,level,industry_code,src"},
				{"index_member", "index_code,index_name,con_code,con_name,in_date,out_date,is_new"},
				{"fund_basic", "ts_code,name,management,custodian,fund_type,found_date,due_date,list_date,issue_date,delist_date,issue_amount,m_fee,c_fee,duration_year,p_value,min_amount,exp_return,benchmark,status,invest_type,type,trustee,purc_startdate,redm_startdate,market"},
				{"fund_nav", "ts_code,ann_date,end_date,unit_nav,accum_nav,accum_div,net_asset,total_netasset,adj_nav"},
				{"fund_div", "ts_code,ann_date,imp_anndate,base_date,div_proc,record_date,ex_date,pay_date,earpay_date,net_ex_date,div_cash,base_unit,ear_distr,ear_amount,account_date,base_year"},
				{"fund_portfolio", "ts_code,ann_date,end_date,symbol,mkv,amount,stk_mkv_ratio,stk_float_ratio"},
				{"fund_daily", "ts_code,trade_date,open,high,low,close,pre_close,change,pct_chg,vol,amount"},
				{"fund_adj", "ts_code,trade_date,adj_factor"},
				{"fut_basic", "ts_code,symbol,exchange,name,fut_code,multiplier,trade_unit,per_unit,quote_unit,quote_unit_desc,d_mode_desc,list_date,delist_date,d_month,last_ddate,trade_time_desc"},
				{"fut_daily", "ts_code,trade_date,pre_close,pre_settle,open,high,low,close,settle,change1,change2,vol,amount,oi,oi_chg,delv_settle"},
				{"fut_holding", "trade_date,symbol,broker,vol,vol_chg,long_hld,long_chg,short_hld,short_chg,exchange"},
				{"fut_wsr", "trade_date,symbol,fut_name,warehouse,wh_id,pre_vol,vol,vol_chg,area,year,grade,brand,place,pd,is_ct,unit,exchange"},
				{"fut_settle", "ts_code,trade_date,settle,trading_fee_rate,trading_fee,delivery_fee,b_hedging_margin_rate,s_hedging_margin_rate,long_margin_rate,short_margin_rate,offset_today_fee,exchange"},
				{"fut_mapping", "ts_code,trade_date,mapping_ts_code"},
				{"opt_basic", "ts_code,exchange,name,per_unit,opt_code,opt_type,call_put,exercise_type,exercise_price,s_month,maturity_date,list_price,list_date,delist_date,last_edate,last_ddate,quote_unit,min_price_chg"},
				{"opt_daily", "ts_code,trade_date,exchange,pre_settle,pre_close,open,high,low,close,settle,vol,amount,oi"},
				{"cb_basic", "ts_code,bond_full_name,bond_short_name,stk_code,stk_short_name,maturity,par,issue_price,issue_size,remain_size,value_date,maturity_date,rate_type,coupon_rate,add_rate,pay_per_year,list_date,exchange,conv_start_date,conv_end_date,first_conv_price,conv_price,rate_clause,put_clause,call_clause,reset_clause,conv_clause,guarantor,guarantee_type,issue_rating,newest_rating,rating_comp"},
				{"cb_issue", "ts_code,ann_date,res_ann_date,plan_issue_size,issue_size,issue_price,issue_type,issue_cost,onl_code,onl_name,onl_date,onl_size,onl_pch_vol,onl_pch_num,onl_pch_excess,onl_winning_rate,shd_ration_code,shd_ration_name,shd_ration_date,shd_ration_record_date,shd_ration_pay_date,shd_ration_price,shd_ration_ratio,shd_ration_size,shd_ration_vol,shd_ration_num,shd_ration_excess,offl_size,offl_deposit,offl_pch_vol,offl_pch_num,offl_pch_excess,offl_winning_rate,lead_underwriter,lead_underwriter_vol"},
				{"cb_daily", "ts_code,trade_date,pre_close,open,high,low,close,change,pct_chg,vol,amount"},
				{"fx_obasic", "ts_code,name,classify,exchange,min_unit,max_unit,pip,pip_cost,traget_spread,min_stop_distance,trading_hours,break_time"},
				{"fx_daily", "ts_code,trade_date,bid_open,bid_close,bid_high,bid_low,ask_open,ask_close,ask_high,ask_low,tick_qty,exchange"},
				{"hk_basic", "ts_code,name,fullname,enname,cn_spell,market,list_status,list_date,delist_date,trade_unit,isin,curr_type"},
				{"hk_daily", "ts_code,trade_date,open,high,low,close,pre_close,change,pct_chg,vol,amount"},
				{"tmt_twincome", "date,item,op_income"},
				{"tmt_twincomedetail", "date,item,symbol,op_income,consop_income"},
				{"bo_monthly", "date,name,list_date,avg_price,month_amount,list_day,p_pc,wom_index,m_ratio,rank"},
				{"bo_weekly", "date,name,avg_price,week_amount,total,list_day,p_pc,wom_index,up_ratio,rank"},
				{"bo_daily", "date,name,avg_price,day_amount,total,list_day,p_pc,wom_index,up_ratio,rank"},
				{"bo_cinema", "date,c_name,aud_count,att_ratio,day_amount,day_showcount,avg_price,p_pc,rank"},
				{"film_record", "rec_no,film_name,rec_org,script_writer,rec_result,rec_area,classified,date_range,ann_date"},
				{"teleplay_record", "name,classify,types,org,report_date,license_key,episodes,shooting_date,prod_cycle,content,pro_opi,dept_opi,remarks"},
				{"shibor", "date,on,1w,2w,1m,3m,6m,9m,1y"},
				{"shibor_quote", "date,bank,on_b,on_a,1w_b,1w_a,2w_b,2w_a,1m_b,1m_a,3m_b,3m_a,6m_b,6m_a,9m_b,9m_a,1y_b,1y_a"},
				{"shibor_lpr", "date,1y"},
				{"libor", "date,curr_type,on,1w,1m,2m,3m,6m,12m"},
				{"hibor", "date,on,1w,2w,1m,2m,3m,6m,12m"},
				{"wz_index", "date,comp_rate,center_rate,micro_rate,cm_rate,sdb_rate,om_rate,aa_rate,m1_rate,m3_rate,m6_rate,m12_rate,long_rate"},
				{"gz_index", "date,d10_rate,m1_rate,m3_rate,m6_rate,m12_rate,long_rate"},
				{"news", "string,content,title,channels"},
				{"major_news", "title,content,pub_time,src"},
				{"cctv_news", "date,title,content"},
				{"anns", "ts_code,ann_date,ann_type,title,content,pub_time"}};

			_types_map={{"stock_basic", "string,string,string,string,string,string,string,string,string,string,string,string,string,string"},
				{"trade_cal", "string,string,string,string"},
				{"namechange", "string,string,string,string,string,string"},
				{"hs_const", "string,string,string,string,string"},
				{"stock_company", "string,string,string,string,string,double,string,string,string,string,string,string,string,int,string,string"},
				{"stk_managers", "string,string,string,string,string,string,string,string,string,string,string,string"},
				{"stk_rewards", "string,string,string,string,string,double,double"},
				{"new_share", "string,string,string,string,string,double,double,double,double,double,double,double"},
				{"daily", "string,string,double,double,double,double,double,double,double,double,double"},
				{"weekly", "string,string,double,double,double,double,double,double,double,double,double"},
				{"monthly", "string,string,double,double,double,double,double,double,double,double,double"},
				{"adj_factor", "string,string,double"},
				{"suspend", "string,string,string,string,string,string"},
				{"daily_basic", "string,string,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double"},
				{"moneyflow", "string,string,int,double,int,double,int,double,int,double,int,double,int,double,int,double,int,double,int,double"},
				{"stk_limit", "string,string,double,double,double"},
				{"limit_list", "string,string,string,double,double,double,double,double,double,string,string,int,double,string"},
				{"moneyflow_hsgt", "string,double,double,double,double,double,double"},
				{"hsgt_top10", "string,string,string,double,double,int,string,double,double,double,double"},
				{"hk_hold", "string,string,string,string,int,double,string"},
				{"ggt_daily", "string,double,double,double,double"},
				{"ggt_monthly", "string,double,double,double,double,double,double,double,double"},
				{"balancesheet", "string,string,string,string,string,string,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,string"},
				{"cashflow", "string,string,string,string,string,string,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,string"},
				{"forecast", "string,string,string,string,double,double,double,double,double,string,string,string"},
				{"express", "string,string,string,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,string,int,string"},
				{"dividend", "string,string,string,string,double,double,double,double,double,string,string,string,string,string,string,double"},
				{"fina_indicator", "string,string,string,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,string"},
				{"fina_audit", "string,string,string,string,double,string,string"},
				{"fina_mainbz", "string,string,string,double,double,double,string,string"},
				{"disclosure_date", "string,string,string,string,string,string"},
				{"ggt_top10", "string,string,string,double,double,string,string,double,double,double,double,double,double,double,double,double,double"},
				{"margin", "string,string,double,double,double,double,double,double,double"},
				{"margin_detail", "string,string,string,double,double,double,double,double,double,double,double"},
				{"top10_holders", "string,string,string,string,double,double"},
				{"top10_floatholders", "string,string,string,string,double"},
				{"top_list", "string,string,string,double,double,double,double,double,double,double,double,double,double,double,string"},
				{"top_inst", "string,string,string,double,double,double,double,double"},
				{"pledge_stat", "string,string,int,double,double,double,double"},
				{"pledge_detail", "string,string,string,double,string,string,string,string,string,double,double,double,double,string"},
				{"repurchase", "string,string,string,string,string,double,double,double,double"},
				{"concept", "string,string,string"},
				{"concept_detail", "string,string,string,string,string,string"},
				{"share_float", "string,string,string,double,double,string,string"},
				{"block_trade", "string,string,double,double,double,string,string"},
				{"stk_account", "string,double,double,double,double"},
				{"stk_account_old", "string,int,int,double,double,double,double,double,double"},
				{"stk_holdernumber", "string,string,string,int"},
				{"stk_holdertrade", "string,string,string,string,string,double,double,double,double,double,double,string,string"},
				{"index_basic", "string,string,string,string,string,string,string,string,double,string,string,string,string"},
				{"index_daily", "string,string,double,double,double,double,double,double,double,double,double"},
				{"index_weekly", "string,string,double,double,double,double,double,double,double,double,double"},
				{"index_monthly", "string,string,double,double,double,double,double,double,double,double,double"},
				{"index_weight", "string,string,string,double"},
				{"index_dailybasic", "string,string,double,double,double,double,double,double,double,double,double,double"},
				{"index_classify", "string,string,string,string,string"},
				{"index_member", "string,string,string,string,string,string,string"},
				{"fund_basic", "string,string,string,string,string,string,string,string,string,string,double,double,double,double,double,double,double,string,string,string,string,string,string,string,string"},
				{"fund_nav", "string,string,string,double,double,double,double,double,double"},
				{"fund_div", "string,string,string,string,string,string,string,string,string,string,double,double,double,double,string,string"},
				{"fund_portfolio", "string,string,string,string,double,double,double,double"},
				{"fund_daily", "string,string,double,double,double,double,double,double,double,double,double"},
				{"fund_adj", "string,string,double"},
				{"fut_basic", "string,string,string,string,string,double,string,double,string,string,string,string,string,string,string,string"},
				{"fut_daily", "string,string,double,double,double,double,double,double,double,double,double,double,double,double,double,double"},
				{"fut_holding", "string,string,string,int,int,int,int,int,int,string"},
				{"fut_wsr", "string,string,string,string,string,int,int,int,string,string,string,string,string,int,string,string,string"},
				{"fut_settle", "string,string,double,double,double,double,double,double,double,double,double,string"},
				{"fut_mapping", "string,string,string"},
				{"opt_basic", "string,string,string,string,string,string,string,string,double,string,string,double,string,string,string,string,string,string"},
				{"opt_daily", "string,string,string,double,double,double,double,double,double,double,double,double,double"},
				{"cb_basic", "string,string,string,string,string,double,double,double,double,double,string,string,string,double,double,int,string,string,string,string,double,double,string,string,string,string,string,string,string,string,string,string"},
				{"cb_issue", "string,string,string,double,double,double,string,double,string,string,string,double,double,int,double,double,string,string,string,string,string,double,double,double,double,int,double,double,double,double,int,double,double,string,double"},
				{"cb_daily", "string,string,double,double,double,double,double,double,double,double,double"},
				{"fx_obasic", "string,string,string,string,double,double,double,double,double,double,string,string"},
				{"fx_daily", "string,string,double,double,double,double,double,double,double,double,int,string"},
				{"hk_basic", "string,string,string,string,string,string,string,string,string,double,string,string"},
				{"hk_daily", "string,string,double,double,double,double,double,double,double,double,double"},
				{"tmt_twincome", "string,string,string"},
				{"tmt_twincomedetail", "string,string,string,string,string"},
				{"bo_monthly", "string,string,string,double,double,int,int,double,double,int"},
				{"bo_weekly", "string,string,double,double,double,int,int,double,double,int"},
				{"bo_daily", "string,string,double,double,double,int,int,double,double,int"},
				{"bo_cinema", "string,string,int,double,double,double,double,double,int"},
				{"film_record", "string,string,string,string,string,string,string,string,string"},
				{"teleplay_record", "string,string,string,string,string,string,string,string,string,string,string,string,string"},
				{"shibor", "string,double,double,double,double,double,double,double,double"},
				{"shibor_quote", "string,string,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double"},
				{"shibor_lpr", "string,double"},
				{"libor", "string,string,double,double,double,double,double,double,double"},
				{"hibor", "string,double,double,double,double,double,double,double,double"},
				{"wz_index", "string,double,double,double,double,double,double,double,double,double,double,double,double"},
				{"gz_index", "string,double,double,double,double,double,double"},
				{"news", "string,string,string,string"},
				{"major_news", "string,string,string,string"},
				{"cctv_news", "string,string,string"},
				{"anns", "string,string,string,string,string,string"}};
		};

	public:
		pro_api(string token) :token(token) {
			initialize_data_map();
			cout << _fields_map.size() << endl;
			cout << _types_map.size() << endl;
			initialize_base_api_req_tree();
			connect_socket();
		};

		//Auto generated API.
		data_store stock_basic(string is_hs ,string list_status ,string exchange){
			ptree parmtree;
			parmtree.put("is_hs",is_hs);
			parmtree.put("list_status",list_status);
			parmtree.put("exchange",exchange);
			return process("stock_basic",parmtree);
		};
		data_store trade_cal(string exchange ,string start_date ,string end_date ,string is_open){
			ptree parmtree;
			parmtree.put("exchange",exchange);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("is_open",is_open);
			return process("trade_cal",parmtree);
		};
		data_store namechange(string ts_code ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("namechange",parmtree);
		};
		data_store hs_const(string hs_type ,string is_new){
			ptree parmtree;
			parmtree.put("hs_type",hs_type);
			parmtree.put("is_new",is_new);
			return process("hs_const",parmtree);
		};
		data_store stock_company(string exchange){
			ptree parmtree;
			parmtree.put("exchange",exchange);
			return process("stock_company",parmtree);
		};
		data_store stk_managers(string ts_code){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			return process("stk_managers",parmtree);
		};
		data_store stk_rewards(string ts_code ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("end_date",end_date);
			return process("stk_rewards",parmtree);
		};
		data_store new_share(string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("new_share",parmtree);
		};
		data_store daily(string ts_code ,string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("daily",parmtree);
		};
		data_store weekly(string ts_code ,string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("weekly",parmtree);
		};
		data_store monthly(string ts_code ,string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("monthly",parmtree);
		};
		data_store adj_factor(string ts_code ,string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("adj_factor",parmtree);
		};
		data_store suspend(string ts_code ,string suspend_date ,string resume_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("suspend_date",suspend_date);
			parmtree.put("resume_date",resume_date);
			return process("suspend",parmtree);
		};
		data_store daily_basic(string ts_code ,string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("daily_basic",parmtree);
		};
		data_store moneyflow(string ts_code ,string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("moneyflow",parmtree);
		};
		data_store stk_limit(string ts_code ,string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("stk_limit",parmtree);
		};
		data_store limit_list(string trade_date ,string ts_code ,string limit_type ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("trade_date",trade_date);
			parmtree.put("ts_code",ts_code);
			parmtree.put("limit_type",limit_type);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("limit_list",parmtree);
		};
		data_store moneyflow_hsgt(string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("moneyflow_hsgt",parmtree);
		};
		data_store hsgt_top10(string ts_code ,string trade_date ,string start_date ,string end_date ,string market_type){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("market_type",market_type);
			return process("hsgt_top10",parmtree);
		};
		data_store hk_hold(string code ,string ts_code ,string trade_date ,string start_date ,string end_date ,string exchange){
			ptree parmtree;
			parmtree.put("code",code);
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("exchange",exchange);
			return process("hk_hold",parmtree);
		};
		data_store ggt_daily(string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("ggt_daily",parmtree);
		};
		data_store ggt_monthly(string month ,string start_month ,string end_month){
			ptree parmtree;
			parmtree.put("month",month);
			parmtree.put("start_month",start_month);
			parmtree.put("end_month",end_month);
			return process("ggt_monthly",parmtree);
		};
		data_store balancesheet(string ts_code ,string ann_date ,string start_date ,string end_date ,string period ,string report_type ,string comp_type){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("ann_date",ann_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("period",period);
			parmtree.put("report_type",report_type);
			parmtree.put("comp_type",comp_type);
			return process("balancesheet",parmtree);
		};
		data_store cashflow(string ts_code ,string ann_date ,string start_date ,string end_date ,string period ,string report_type ,string comp_type){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("ann_date",ann_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("period",period);
			parmtree.put("report_type",report_type);
			parmtree.put("comp_type",comp_type);
			return process("cashflow",parmtree);
		};
		data_store forecast(string ts_code ,string ann_date ,string start_date ,string end_date ,string period ,string type){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("ann_date",ann_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("period",period);
			parmtree.put("type",type);
			return process("forecast",parmtree);
		};
		data_store express(string ts_code ,string ann_date ,string start_date ,string end_date ,string period){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("ann_date",ann_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("period",period);
			return process("express",parmtree);
		};
		data_store dividend(string ts_code ,string ann_date ,string record_date ,string ex_date ,string imp_ann_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("ann_date",ann_date);
			parmtree.put("record_date",record_date);
			parmtree.put("ex_date",ex_date);
			parmtree.put("imp_ann_date",imp_ann_date);
			return process("dividend",parmtree);
		};
		data_store fina_indicator(string ts_code ,string ann_date ,string start_date ,string end_date ,string period){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("ann_date",ann_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("period",period);
			return process("fina_indicator",parmtree);
		};
		data_store fina_audit(string ts_code ,string ann_date ,string start_date ,string end_date ,string period){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("ann_date",ann_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("period",period);
			return process("fina_audit",parmtree);
		};
		data_store fina_mainbz(string ts_code ,string period ,string type ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("period",period);
			parmtree.put("type",type);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("fina_mainbz",parmtree);
		};
		data_store disclosure_date(string ts_code ,string end_date ,string pre_date ,string actual_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("end_date",end_date);
			parmtree.put("pre_date",pre_date);
			parmtree.put("actual_date",actual_date);
			return process("disclosure_date",parmtree);
		};
		data_store ggt_top10(string ts_code ,string trade_date ,string start_date ,string end_date ,string market_type){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("market_type",market_type);
			return process("ggt_top10",parmtree);
		};
		data_store margin(string trade_date ,string exchange_id ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("trade_date",trade_date);
			parmtree.put("exchange_id",exchange_id);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("margin",parmtree);
		};
		data_store margin_detail(string trade_date ,string ts_code ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("trade_date",trade_date);
			parmtree.put("ts_code",ts_code);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("margin_detail",parmtree);
		};
		data_store top10_holders(string ts_code ,string period ,string ann_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("period",period);
			parmtree.put("ann_date",ann_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("top10_holders",parmtree);
		};
		data_store top10_floatholders(string ts_code ,string period ,string ann_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("period",period);
			parmtree.put("ann_date",ann_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("top10_floatholders",parmtree);
		};
		data_store top_list(string trade_date ,string ts_code){
			ptree parmtree;
			parmtree.put("trade_date",trade_date);
			parmtree.put("ts_code",ts_code);
			return process("top_list",parmtree);
		};
		data_store top_inst(string trade_date ,string ts_code){
			ptree parmtree;
			parmtree.put("trade_date",trade_date);
			parmtree.put("ts_code",ts_code);
			return process("top_inst",parmtree);
		};
		data_store pledge_stat(string ts_code){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			return process("pledge_stat",parmtree);
		};
		data_store pledge_detail(string ts_code){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			return process("pledge_detail",parmtree);
		};
		data_store repurchase(string ann_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ann_date",ann_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("repurchase",parmtree);
		};
		data_store concept(string src){
			ptree parmtree;
			parmtree.put("src",src);
			return process("concept",parmtree);
		};
		data_store concept_detail(string id ,string ts_code){
			ptree parmtree;
			parmtree.put("id",id);
			parmtree.put("ts_code",ts_code);
			return process("concept_detail",parmtree);
		};
		data_store share_float(string ts_code ,string ann_date ,string float_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("ann_date",ann_date);
			parmtree.put("float_date",float_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("share_float",parmtree);
		};
		data_store block_trade(string ts_code ,string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("block_trade",parmtree);
		};
		data_store stk_account(string date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("date",date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("stk_account",parmtree);
		};
		data_store stk_account_old(string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("stk_account_old",parmtree);
		};
		data_store stk_holdernumber(string ts_code ,string enddate ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("enddate",enddate);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("stk_holdernumber",parmtree);
		};
		data_store stk_holdertrade(string ts_code ,string ann_date ,string start_date ,string end_date ,string trade_type ,string holder_type){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("ann_date",ann_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("trade_type",trade_type);
			parmtree.put("holder_type",holder_type);
			return process("stk_holdertrade",parmtree);
		};
		data_store index_basic(string market ,string publisher ,string category){
			ptree parmtree;
			parmtree.put("market",market);
			parmtree.put("publisher",publisher);
			parmtree.put("category",category);
			return process("index_basic",parmtree);
		};
		data_store index_daily(string ts_code ,string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("index_daily",parmtree);
		};
		data_store index_weekly(string ts_code ,string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("index_weekly",parmtree);
		};
		data_store index_monthly(string ts_code ,string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("index_monthly",parmtree);
		};
		data_store index_weight(string index_code ,string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("index_code",index_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("index_weight",parmtree);
		};
		data_store index_dailybasic(string trade_date ,string ts_code ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("trade_date",trade_date);
			parmtree.put("ts_code",ts_code);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("index_dailybasic",parmtree);
		};
		data_store index_classify(string index_code ,string level ,string src){
			ptree parmtree;
			parmtree.put("index_code",index_code);
			parmtree.put("level",level);
			parmtree.put("src",src);
			return process("index_classify",parmtree);
		};
		data_store index_member(string index_code ,string ts_code ,string is_new){
			ptree parmtree;
			parmtree.put("index_code",index_code);
			parmtree.put("ts_code",ts_code);
			parmtree.put("is_new",is_new);
			return process("index_member",parmtree);
		};
		data_store fund_basic(string market){
			ptree parmtree;
			parmtree.put("market",market);
			return process("fund_basic",parmtree);
		};
		data_store fund_nav(string ts_code ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("end_date",end_date);
			return process("fund_nav",parmtree);
		};
		data_store fund_div(string ann_date ,string ex_date ,string pay_date ,string ts_code){
			ptree parmtree;
			parmtree.put("ann_date",ann_date);
			parmtree.put("ex_date",ex_date);
			parmtree.put("pay_date",pay_date);
			parmtree.put("ts_code",ts_code);
			return process("fund_div",parmtree);
		};
		data_store fund_portfolio(string ts_code){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			return process("fund_portfolio",parmtree);
		};
		data_store fund_daily(string ts_code ,string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("fund_daily",parmtree);
		};
		data_store fund_adj(string ts_code ,string trade_date ,string start_date ,string end_date ,string offset ,string limit){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("offset",offset);
			parmtree.put("limit",limit);
			return process("fund_adj",parmtree);
		};
		data_store fut_basic(string exchange ,string fut_type){
			ptree parmtree;
			parmtree.put("exchange",exchange);
			parmtree.put("fut_type",fut_type);
			return process("fut_basic",parmtree);
		};
		data_store fut_daily(string trade_date ,string ts_code ,string exchange ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("trade_date",trade_date);
			parmtree.put("ts_code",ts_code);
			parmtree.put("exchange",exchange);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("fut_daily",parmtree);
		};
		data_store fut_holding(string trade_date ,string symbol ,string start_date ,string end_date ,string exchange){
			ptree parmtree;
			parmtree.put("trade_date",trade_date);
			parmtree.put("symbol",symbol);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("exchange",exchange);
			return process("fut_holding",parmtree);
		};
		data_store fut_wsr(string trade_date ,string symbol ,string start_date ,string end_date ,string exchange){
			ptree parmtree;
			parmtree.put("trade_date",trade_date);
			parmtree.put("symbol",symbol);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("exchange",exchange);
			return process("fut_wsr",parmtree);
		};
		data_store fut_settle(string trade_date ,string ts_code ,string start_date ,string end_date ,string exchange){
			ptree parmtree;
			parmtree.put("trade_date",trade_date);
			parmtree.put("ts_code",ts_code);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("exchange",exchange);
			return process("fut_settle",parmtree);
		};
		data_store fut_mapping(string ts_code ,string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("fut_mapping",parmtree);
		};
		data_store opt_basic(string exchange ,string call_put){
			ptree parmtree;
			parmtree.put("exchange",exchange);
			parmtree.put("call_put",call_put);
			return process("opt_basic",parmtree);
		};
		data_store opt_daily(string ts_code ,string trade_date ,string start_date ,string end_date ,string exchange){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("exchange",exchange);
			return process("opt_daily",parmtree);
		};
		data_store cb_basic(string ts_code ,string list_date ,string exchange){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("list_date",list_date);
			parmtree.put("exchange",exchange);
			return process("cb_basic",parmtree);
		};
		data_store cb_issue(string ts_code ,string ann_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("ann_date",ann_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("cb_issue",parmtree);
		};
		data_store cb_daily(string ts_code ,string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("cb_daily",parmtree);
		};
		data_store fx_obasic(string exchange ,string classify ,string ts_code){
			ptree parmtree;
			parmtree.put("exchange",exchange);
			parmtree.put("classify",classify);
			parmtree.put("ts_code",ts_code);
			return process("fx_obasic",parmtree);
		};
		data_store fx_daily(string ts_code ,string trade_date ,string start_date ,string end_date ,string exchange){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("exchange",exchange);
			return process("fx_daily",parmtree);
		};
		data_store hk_basic(string ts_code ,string list_status){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("list_status",list_status);
			return process("hk_basic",parmtree);
		};
		data_store hk_daily(string ts_code ,string trade_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("trade_date",trade_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("hk_daily",parmtree);
		};
		data_store tmt_twincome(string date ,string item ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("date",date);
			parmtree.put("item",item);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("tmt_twincome",parmtree);
		};
		data_store tmt_twincomedetail(string date ,string item ,string symbol ,string start_date ,string end_date ,string source){
			ptree parmtree;
			parmtree.put("date",date);
			parmtree.put("item",item);
			parmtree.put("symbol",symbol);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("source",source);
			return process("tmt_twincomedetail",parmtree);
		};
		data_store bo_monthly(string date){
			ptree parmtree;
			parmtree.put("date",date);
			return process("bo_monthly",parmtree);
		};
		data_store bo_weekly(string date){
			ptree parmtree;
			parmtree.put("date",date);
			return process("bo_weekly",parmtree);
		};
		data_store bo_daily(string date){
			ptree parmtree;
			parmtree.put("date",date);
			return process("bo_daily",parmtree);
		};
		data_store bo_cinema(string date){
			ptree parmtree;
			parmtree.put("date",date);
			return process("bo_cinema",parmtree);
		};
		data_store film_record(string ann_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ann_date",ann_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("film_record",parmtree);
		};
		data_store teleplay_record(string report_date ,string start_date ,string end_date ,string org ,string name){
			ptree parmtree;
			parmtree.put("report_date",report_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("org",org);
			parmtree.put("name",name);
			return process("teleplay_record",parmtree);
		};
		data_store shibor(string date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("date",date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("shibor",parmtree);
		};
		data_store shibor_quote(string date ,string start_date ,string end_date ,string bank){
			ptree parmtree;
			parmtree.put("date",date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("bank",bank);
			return process("shibor_quote",parmtree);
		};
		data_store shibor_lpr(string date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("date",date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("shibor_lpr",parmtree);
		};
		data_store libor(string date ,string start_date ,string end_date ,string curr_type){
			ptree parmtree;
			parmtree.put("date",date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("curr_type",curr_type);
			return process("libor",parmtree);
		};
		data_store hibor(string date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("date",date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("hibor",parmtree);
		};
		data_store wz_index(string date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("date",date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("wz_index",parmtree);
		};
		data_store gz_index(string date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("date",date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("gz_index",parmtree);
		};
		data_store news(string start_date ,string end_date ,string src){
			ptree parmtree;
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			parmtree.put("src",src);
			return process("news",parmtree);
		};
		data_store major_news(string src ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("src",src);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("major_news",parmtree);
		};
		data_store cctv_news(string date){
			ptree parmtree;
			parmtree.put("date",date);
			return process("cctv_news",parmtree);
		};
		data_store anns(string ts_code ,string ann_date ,string start_date ,string end_date){
			ptree parmtree;
			parmtree.put("ts_code",ts_code);
			parmtree.put("ann_date",ann_date);
			parmtree.put("start_date",start_date);
			parmtree.put("end_date",end_date);
			return process("anns",parmtree);
		};
		
	};


};

#endif