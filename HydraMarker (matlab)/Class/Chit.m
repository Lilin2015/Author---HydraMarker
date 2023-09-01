classdef Chit
    
    methods (Static)
        function hit_map = hit_partially(state,pool)
            hit_map = Chit.hit(state,pool,true);
        end
        function hit_map = hit_fully(state,pool)
            hit_map = Chit.hit(state,pool,false);
        end
        function hit_map = tag_hit(state,tag,nanflag)
            % hit_map(m,n)=1 means the tag hits the region of state that start from the element sta(m,n) (top-left)
            [inst_M,inst_N] = size(tag);

            trans_m = floor((inst_M+1)/2)-1;
            trans_n = floor((inst_N+1)/2)-1;
                    
            SE = tag;    

            strel0 = strel(SE==0);
            strel1 = strel(SE==1);
            strelv = strel(SE~=-1);

            if any(strel0.Neighborhood,'all')
                SE0 = translate(strel0,[trans_m trans_n]);
                ER0 = imerode(state.val==0|(state.val==-1&nanflag),SE0);
            else
                ER0 = ones(size(state.val));
            end

            if any(strel1.Neighborhood,'all')
                SE1 = translate(strel1,[trans_m trans_n]);
                ER1 = imerode(state.val==1|(state.val==-1&nanflag),SE1);
            else
                ER1 = ones(size(state.val));
            end

            if any(strelv.Neighborhood,'all')
                SEv = translate(strelv,[trans_m trans_n]);
                ERv = imdilate(state.val==-2,reflect(SEv));
            else
                ERv = ones(size(state.val));
            end

            hit_map = double(ER0 & ER1);
            hit_map(ERv) = -2;
            
            % remove the regions near boundaries
            hit_map(max(1,end-inst_M+2):end,:) = [];
            hit_map(:,max(1,end-inst_N+2):end) = [];
        end
    end
        
    methods (Static, Access=private)
        %   nanflag indicates whether the unknowns in state are treated as both 0 and 1.
        %   If true, partially hit, these elements are always hit.
        %   If false, fully hit, these elements are hit only if the corresponding tag elements
        %   are NaN (not belongs to the tag).
        function hit_map = hit(state,pool,nanflag)
            %mustBeA(state,'Dfield');
            %mustBeA(pool,'Dpool');
            mustBeMember(nanflag,[0,1]);
             
            [M,N] = size(state.val);
            snum = size(pool.val,1);
            
            hit_map = cell(snum,4);
            for t = 1 : snum
                for k = 1 : 4
                    [inst_M,inst_N,inst_num] = size(pool.val{t,k});
                    hit_map{t,k} = zeros(M,N,inst_num);
                    % example. hit_map{t,2}(m,n,i)=1 means the i-th tag of pool{t,2}
                    % hits the region of state that start from the element sta(m,n) (top-left).

                    trans_m = floor((inst_M+1)/2)-1;
                    trans_n = floor((inst_N+1)/2)-1;
                    
                    for i = 1 : inst_num
                        SE = pool.val{t,k}(:,:,i);    
                        
                        strel0 = strel(SE==0);
                        strel1 = strel(SE==1);
                        strelv = strel(SE~=-1);
                        
                        if any(strel0.Neighborhood,'all')
                            SE0 = translate(strel0,[trans_m trans_n]);
                            ER0 = imerode(state.val==0|(state.val==-1&nanflag),SE0);
                        else
                            ER0 = zeros(size(state.val));
                        end
                        
                        if any(strel1.Neighborhood,'all')
                            SE1 = translate(strel1,[trans_m trans_n]);
                            ER1 = imerode(state.val==1|(state.val==-1&nanflag),SE1);
                        else
                            ER1 = zeros(size(state.val));
                        end
                        
                        if any(strelv.Neighborhood,'all')
                            SEv = translate(strelv,[trans_m trans_n]);
                            ERv = imdilate(state.val==-2,reflect(SEv));
                        else
                            ERv = zeros(size(state.val));
                        end
                        
                        hit_in = double(ER0 & ER1);
                        hit_in(ERv) = -2;
                        
%                         hit_hollow = imdilate(state.val==-2,reflect(SEv));
%                         hit_in(hit_hollow) = -2;
                        hit_map{t,k}(:,:,i) = hit_in;
                    end  
                    % remove the regions near boundaries
                    
                    hit_map{t,k}(max(1,end-inst_M+2):end,:,:) = [];
                    hit_map{t,k}(:,max(1,end-inst_N+2):end,:) = [];
                end

            end % end for t = 1 : size(pool,1)
            hit_map = Dhit_map(hit_map);
        end % function
        
    end % method
end

