#ifndef _SDP_H_
#define _SDP_H_

#include "../list.h"

#ifdef __cplusplus
extern "C"
{
#endif


	typedef struct sdp_bandwidth_t{
		char *b_bwtype;
		char *b_bandwidth;
	}sdp_bandwidth_t;

	/**
	* Structure for referencing time description header.
	* @defvar sdp_time_descr_t
	*/
	typedef struct sdp_time_descr_t{
		char *t_start_time;
		char *t_stop_time;
		list_t *r_repeats;		/* list of char * */
	}sdp_time_descr_t;

	/**
	* Structure for referencing key header.
	* @defvar sdp_key_t
	*/
	typedef struct sdp_key_t{
		char *k_keytype;		/* "prompt", "clear", "base64", "uri" */
		char *k_keydata;		/* key data for "clear" and "base64", uri for "uri" */
	}sdp_key_t;

	/**
	* Structure for referencing an attribute header.
	* @defvar sdp_attribute_t
	*/
	typedef struct  sdp_attribute_t{
		char *a_att_field;
		char *a_att_value;		/* optional */
	}sdp_attribute_t;

	/**
	* Structure for referencing a connection header.
	* @defvar sdp_connection_t
	*/
	typedef struct sdp_connection_t{
		char *c_nettype;
		char *c_addrtype;
		char *c_addr;
		char *c_addr_multicast_ttl;
		char *c_addr_multicast_int;
	}sdp_connection_t;

	/**
	* Structure for referencing a media header.
	* @defvar sdp_media_t
	*/
	typedef struct sdp_media_t{
		char *m_media;
		char *m_port;
		char *m_number_of_port;
		char *m_proto;
		list_t *m_payloads;

		char *i_info;
		list_t *c_connections;	/* list of sdp_connection_t * */
		list_t *b_bandwidths;	/* list of sdp_bandwidth_t * */
		list_t *a_attributes;	/* list of sdp_attribute_t * */
		sdp_key_t *k_key;
	}sdp_media_t;

	/**
	* Structure for referencing a SDP packet.
	* @defvar sdp_t
	*/
	typedef struct sdp_t{
		char *v_version;
		char *o_username;
		char *o_sess_id;
		char *o_sess_version;
		char *o_nettype;
		char *o_addrtype;
		char *o_addr;
		char *s_name;
		char *i_info;
		char *u_uri;
		list_t *e_emails;		/* list of char * */
		list_t *p_phones;		/* list of char * */
		sdp_connection_t *c_connection;
		list_t *b_bandwidths;	/* list of sdp_bandwidth_t * */
		list_t *t_descrs;		/* list of sdp_time_descr_t * */
		char *z_adjustments;
		sdp_key_t *k_key;
		list_t *a_attributes;	/* list of sdp_attribute_t * */
		list_t *m_medias;		/* list of sdp_media_t * */
	}sdp_t;


	
	int sdp_bandwidth_init(sdp_bandwidth_t **elem);

	void sdp_bandwidth_free (sdp_bandwidth_t * elem);

	int sdp_time_descr_init (sdp_time_descr_t ** elem);
	
	void sdp_time_descr_free (sdp_time_descr_t * elem);
	
	int sdp_key_init (sdp_key_t ** elem);
	
	void sdp_key_free (sdp_key_t * elem);

	
	int sdp_attribute_init (sdp_attribute_t ** elem);
	
	void sdp_attribute_free (sdp_attribute_t * elem);
	
	int sdp_connection_init (sdp_connection_t ** elem);
	
	void sdp_connection_free (sdp_connection_t * elem);

	
	int sdp_media_init (sdp_media_t ** elem);
	
	void sdp_media_free (sdp_media_t * elem);
	
	int sdp_init (sdp_t ** sdp);
		
	void sdp_free (sdp_t * sdp);

	int sdp_parse (sdp_t * sdp, const char *buf);
	
	int sdp_2char (sdp_t * sdp, char **dest);
	
	int sdp_clone (sdp_t * sdp, sdp_t ** dest);
	
	int sdp_v_version_set (sdp_t * sdp, char *value);
	
	char *sdp_v_version_get (sdp_t * sdp);

	int sdp_o_origin_set (sdp_t * sdp, char *username, char *sess_id,
		char *sess_version, char *nettype,
		char *addrtype, char *addr);

	char *sdp_o_username_get (sdp_t * sdp);
	char *sdp_o_sess_id_get (sdp_t * sdp);
	char *sdp_o_sess_version_get (sdp_t * sdp);
	char *sdp_o_nettype_get (sdp_t * sdp);
	char *sdp_o_addrtype_get (sdp_t * sdp);
	char *sdp_o_addr_get (sdp_t * sdp);
	int sdp_s_name_set (sdp_t * sdp, char *value);
	char *sdp_s_name_get (sdp_t * sdp);
	int sdp_i_info_set (sdp_t * sdp, int pos_media, char *value);
	char *sdp_i_info_get (sdp_t * sdp, int pos_media);
	int sdp_u_uri_set (sdp_t * sdp, char *value);
	char *sdp_u_uri_get (sdp_t * sdp);
	int sdp_e_email_add (sdp_t * sdp, char *value);
	char *sdp_e_email_get (sdp_t * sdp, int pos);
	int sdp_p_phone_add (sdp_t * sdp, char *value);
	char *sdp_p_phone_get (sdp_t * sdp, int pos);
	
	int sdp_c_connection_add (sdp_t * sdp, int pos_media,
		char *nettype, char *addrtype,
		char *addr, char *addr_multicast_ttl,
		char *addr_multicast_int);

	/* this method should be internal only... */
	sdp_connection_t *sdp_connection_get (sdp_t * sdp, int pos_media, int pos);
	
	char *sdp_c_nettype_get (sdp_t * sdp, int pos_media, int pos);
	char *sdp_c_addrtype_get (sdp_t * sdp, int pos_media, int pos);
	
	char *sdp_c_addr_get (sdp_t * sdp, int pos_media, int pos);
	
	char *sdp_c_addr_multicast_ttl_get (sdp_t * sdp, int pos_media, int pos);
	
	char *sdp_c_addr_multicast_int_get (sdp_t * sdp, int pos_media, int pos);
	
	int sdp_b_bandwidth_add (sdp_t * sdp, int pos_media,
		char *bwtype, char *bandwidth);
	
	sdp_bandwidth_t *sdp_bandwidth_get (sdp_t * sdp, int pos_media, int pos);
	
	char *sdp_b_bwtype_get (sdp_t * sdp, int pos_media, int pos);
	
	char *sdp_b_bandwidth_get (sdp_t * sdp, int pos_media, int pos);
	
	int sdp_t_time_descr_add (sdp_t * sdp, char *start, char *stop);
	
	char *sdp_t_start_time_get (sdp_t * sdp, int pos_td);
	
	char *sdp_t_stop_time_get (sdp_t * sdp, int pos_td);
	
	int sdp_r_repeat_add (sdp_t * sdp, int pos_time_descr, char *value);
	char *sdp_r_repeat_get (sdp_t * sdp, int pos_time_descr, int pos_repeat);
	
	int sdp_z_adjustments_set (sdp_t * sdp, char *value);
	
	char *sdp_z_adjustments_get (sdp_t * sdp);
	
	int sdp_k_key_set (sdp_t * sdp, int pos_media, char *keytype,
		char *keydata);
	
	char *sdp_k_keytype_get (sdp_t * sdp, int pos_media);
		char *sdp_k_keydata_get (sdp_t * sdp, int pos_media);
	
	int sdp_a_attribute_add (sdp_t * sdp, int pos_media, char *att_field,
		char *att_value);
	
	sdp_attribute_t *sdp_attribute_get (sdp_t * sdp, int pos_media, int pos);
	
	char *sdp_a_att_field_get (sdp_t * sdp, int pos_media, int pos);
	
	char *sdp_a_att_value_get (sdp_t * sdp, int pos_media, int pos);
	
	int sdp_endof_media (sdp_t * sdp, int pos);
	
	int sdp_m_media_add (sdp_t * sdp, char *media,
		char *port, char *number_of_port, char *proto);
	
	char *sdp_m_media_get (sdp_t * sdp, int pos_media);
	
	char *sdp_m_port_get (sdp_t * sdp, int pos_media);
	
	char *sdp_m_number_of_port_get (sdp_t * sdp, int pos_media);
	
	char *sdp_m_proto_get (sdp_t * sdp, int pos_media);
	int sdp_m_payload_add (sdp_t * sdp, int pos_media, char *payload);
	
	char *sdp_m_payload_get (sdp_t * sdp, int pos_media, int pos);

    char * sstrncpy (char *dest, const char *src, int length);


#ifdef __cplusplus
}
#endif

#endif

