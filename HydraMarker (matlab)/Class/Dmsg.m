classdef Dmsg
    
    properties (SetAccess=private)
        str;
        str_list = {'empty';
                    'tag_repeated';
                    'pool_updated';
                    'state_complete';
                    's_state_no_support';
                    'filling_sugst';
                    'procs_complete';
                    'modify'};
    end
    
    
    
    methods
   
        function obj = Dmsg(str)
            mustBeMember(str,obj.str_list);
            obj.str = str;
        end
        
    end
end
